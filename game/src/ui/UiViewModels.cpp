// Intended function: imported ui implementation for UiViewModels; preserves the agent-authored subsystem contract for later integration/debugging.
#include "ui/UiViewModels.hpp"

#include "world/Block.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <tuple>

namespace elysium {
namespace {

bool requiresElectricPower(MachineType type) {
    switch(type) {
        case MachineType::BurnerGenerator:
        case MachineType::BatteryBank:
        case MachineType::StorageCrate:
        case MachineType::Furnace:
            return false;
        default:
            return true;
    }
}

bool processMachine(MachineType type) {
    switch(type) {
        case MachineType::Furnace:
        case MachineType::AlloyCrucible:
        case MachineType::Refinery:
        case MachineType::Crusher:
        case MachineType::ChemicalVat:
        case MachineType::Fabricator:
        case MachineType::ArcSmelter:
            return true;
        default:
            return false;
    }
}

bool logisticsMachine(MachineType type) {
    return type==MachineType::Conveyor || type==MachineType::Sorter || type==MachineType::CargoLoader;
}

int localCount(const SurfaceMachineObject& machine, int itemId) {
    const auto it=std::find_if(machine.inventory.begin(),machine.inventory.end(),
        [itemId](const SurfaceItemStack& stack){return stack.itemId==itemId;});
    return it==machine.inventory.end()?0:it->count;
}

int availableInput(const SurfaceInfrastructure& infrastructure,
                   const SurfaceMachineObject& machine,
                   int itemId) {
    int total=localCount(machine,itemId);
    if(machine.powerNetworkId==0) return total;
    for(const auto& other:infrastructure.objects()) {
        if(other.stableId==machine.stableId || other.type!=MachineType::NetworkStorage ||
           !other.enabled || !other.powered || other.powerNetworkId!=machine.powerNetworkId) continue;
        total+=localCount(other,itemId);
    }
    return total;
}

bool canAcceptLocal(const SurfaceMachineObject& machine, int itemId, int count) {
    if(count<=0) return true;
    const auto it=std::find_if(machine.inventory.begin(),machine.inventory.end(),
        [itemId](const SurfaceItemStack& stack){return stack.itemId==itemId;});
    if(it!=machine.inventory.end()) return it->count<=SurfaceIndustrySystem::MaxStackCount-count;
    return static_cast<int>(machine.inventory.size())<SurfaceIndustrySystem::inventoryCapacity(machine) &&
           count<=SurfaceIndustrySystem::MaxStackCount;
}

bool canRouteOutput(const SurfaceInfrastructure& infrastructure,
                    const SurfaceMachineObject& machine,
                    int itemId,
                    int count) {
    if(canAcceptLocal(machine,itemId,count)) return true;
    if(machine.powerNetworkId==0) return false;
    for(const auto& other:infrastructure.objects()) {
        if(other.stableId==machine.stableId || other.type!=MachineType::NetworkStorage ||
           !other.enabled || !other.powered || other.powerNetworkId!=machine.powerNetworkId) continue;
        if(canAcceptLocal(other,itemId,count)) return true;
    }
    return false;
}

std::string boolWord(bool value, std::string_view yes, std::string_view no) {
    return std::string(value?yes:no);
}

std::string floatText(float value, int precision=1) {
    std::ostringstream out;
    out<<std::fixed<<std::setprecision(precision)<<value;
    return out.str();
}

std::string percentText(float value) {
    return floatText(std::clamp(value,0.0f,1.0f)*100.0f,0)+"%";
}

std::string inventoryText(const SurfaceMachineObject& machine) {
    if(machine.inventory.empty()) return "empty";
    std::ostringstream out;
    bool first=true;
    for(const auto& stack:machine.inventory) {
        if(stack.count<=0) continue;
        if(!first) out<<", ";
        first=false;
        out<<industryItemName(stack.itemId)<<" x"<<stack.count;
    }
    return first?"empty":out.str();
}

void addFact(WhyInspectorModel& out, std::string label, std::string value) {
    out.facts.push_back({std::move(label),std::move(value)});
}

void addDiagnostic(WhyInspectorModel& out,
                   std::string key,
                   UiSeverity severity,
                   std::string category,
                   std::uint64_t sourceStableId,
                   std::string summary,
                   std::string cause,
                   std::string remediation) {
    out.diagnostics.push_back({std::move(key),severity,std::move(category),sourceStableId,
                               std::move(summary),std::move(cause),std::move(remediation)});
}

int severityRank(UiSeverity severity) {
    return static_cast<int>(severity);
}

void sortDiagnostics(std::vector<UiDiagnostic>& diagnostics) {
    std::stable_sort(diagnostics.begin(),diagnostics.end(),[](const auto& a,const auto& b){
        if(severityRank(a.severity)!=severityRank(b.severity)) return severityRank(a.severity)>severityRank(b.severity);
        if(a.category!=b.category) return a.category<b.category;
        if(a.sourceStableId!=b.sourceStableId) return a.sourceStableId<b.sourceStableId;
        return a.key<b.key;
    });
}

const SurfaceRecipeDefinition* selectedOrActiveRecipe(const SurfaceMachineObject& machine) {
    if(machine.activeRecipeId!=0) {
        const auto* active=surfaceRecipe(static_cast<SurfaceRecipeId>(machine.activeRecipeId));
        if(active && active->machine==machine.type) return active;
    }
    if(machine.selectedRecipeId!=0) {
        const auto* selected=surfaceRecipe(static_cast<SurfaceRecipeId>(machine.selectedRecipeId));
        if(selected && selected->machine==machine.type) return selected;
    }
    return nullptr;
}

void diagnoseProcessMachine(WhyInspectorModel& out,
                            const SurfaceInfrastructure& infrastructure,
                            const SurfaceMachineObject& machine) {
    const auto* recipe=selectedOrActiveRecipe(machine);
    if(recipe) {
        addFact(out,"Recipe",recipe->name.empty()?"unknown":std::string(recipe->name));
        const float progress=recipe->processSeconds>0.0f?std::clamp(machine.processProgressSeconds/recipe->processSeconds,0.0f,1.0f):0.0f;
        addFact(out,"Batch",machine.activeRecipeId!=0?(percentText(progress)+" complete"):"waiting to reserve inputs");

        std::vector<std::string> missing;
        for(int i=0;i<recipe->inputCount;++i) {
            const auto ingredient=recipe->inputs[static_cast<std::size_t>(i)];
            const int have=availableInput(infrastructure,machine,ingredient.itemId);
            if(have<ingredient.count) {
                std::ostringstream text;
                text<<industryItemName(ingredient.itemId)<<" "<<have<<"/"<<ingredient.count;
                missing.push_back(text.str());
            }
        }
        if(machine.type==MachineType::Furnace && machine.fuelSeconds<=0.0f) {
            int recipeCoal=0;
            for(int i=0;i<recipe->inputCount;++i)
                if(recipe->inputs[static_cast<std::size_t>(i)].itemId==static_cast<int>(BlockType::CoalOre))
                    recipeCoal+=recipe->inputs[static_cast<std::size_t>(i)].count;
            const int coal=availableInput(infrastructure,machine,static_cast<int>(BlockType::CoalOre));
            if(coal<recipeCoal+1) missing.push_back("Coal Ore fuel "+std::to_string(std::max(0,coal-recipeCoal))+"/1");
        }
        if(!missing.empty() && machine.activeRecipeId==0) {
            std::ostringstream list;
            for(std::size_t i=0;i<missing.size();++i) { if(i) list<<", "; list<<missing[i]; }
            addDiagnostic(out,"industry.missing_input",UiSeverity::Advisory,"industry",machine.stableId,
                          "Production is waiting for input",list.str(),
                          "Deliver the named material locally or through powered Network Storage on this base network.");
        }
        if(!canRouteOutput(infrastructure,machine,recipe->outputItemId,recipe->outputCount)) {
            addDiagnostic(out,"industry.output_full",UiSeverity::Advisory,"industry",machine.stableId,
                          "Production output is blocked",
                          std::string(industryItemName(recipe->outputItemId))+" has no accepting local or same-network storage capacity.",
                          "Free machine/storage capacity or connect powered Network Storage in the same local network.");
        }
    } else {
        const auto recipes=surfaceRecipesForMachine(machine.type);
        if(!recipes.empty() && machine.activeRecipeId==0) {
            addFact(out,"Recipe","AUTO // first complete input set");
            bool anyReady=false;
            for(const auto* candidate:recipes) {
                bool ready=true;
                for(int i=0;i<candidate->inputCount;++i) {
                    const auto ingredient=candidate->inputs[static_cast<std::size_t>(i)];
                    if(availableInput(infrastructure,machine,ingredient.itemId)<ingredient.count) {ready=false;break;}
                }
                if(ready) {anyReady=true;break;}
            }
            if(!anyReady) {
                addDiagnostic(out,"industry.no_ready_recipe",UiSeverity::Advisory,"industry",machine.stableId,
                              "No recipe can currently start",
                              "Automatic recipe selection found no complete input set.",
                              "Load a valid recipe's ingredients or select a recipe to see its exact missing inputs.");
            }
        }
    }

    if(machine.type==MachineType::Furnace)
        addFact(out,"Fuel",floatText(machine.fuelSeconds,1)+" s");
}

void diagnosePower(WhyInspectorModel& out,
                   const SurfaceMachineObject& machine,
                   const UiDiagnosticContext& context) {
    if(!machine.enabled) {
        addDiagnostic(out,"machine.disabled",UiSeverity::Advisory,"machine",machine.stableId,
                      "Machine is disabled","Its enabled policy is off.","Enable the machine when you want this function to run.");
        return;
    }
    if(!requiresElectricPower(machine.type)) return;
    if(machine.powered) return;

    UiSeverity severity=UiSeverity::Advisory;
    if(machine.type==MachineType::AtmosphereUnit) severity=UiSeverity::High;
    if((machine.type==MachineType::Turret || machine.type==MachineType::SensorMast || machine.type==MachineType::ShieldPylon) &&
       context.defense.hostilesDetected>0) severity=UiSeverity::High;
    addDiagnostic(out,"power.unpowered",severity,"power",machine.stableId,
                  "Machine has no supplied power",
                  context.power.shedLoads>0?"The base is shedding loads under a brownout.":"The machine is disconnected, disabled by network state, or has no generation available.",
                  "Restore local generation/storage or reduce higher-priority demand; verify this machine is inside the intended base network.");
}

std::string alertFingerprint(const UiAlert& alert) {
    return std::to_string(static_cast<int>(alert.severity))+"\n"+alert.category+"\n"+alert.title+"\n"+alert.detail+"\n"+alert.remediation;
}

void addAlert(std::map<std::string,UiAlert,std::less<>>& alerts, UiAlert alert) {
    const auto it=alerts.find(alert.key);
    if(it==alerts.end() || severityRank(alert.severity)>severityRank(it->second.severity)) alerts[alert.key]=std::move(alert);
}

const char* designationName(UiDesignationKind designation) {
    switch(designation) {
        case UiDesignationKind::Mine: return "MINE";
        case UiDesignationKind::Channel: return "CHANNEL";
        case UiDesignationKind::Construct: return "CONSTRUCT";
        case UiDesignationKind::Deconstruct: return "DECONSTRUCT";
        case UiDesignationKind::Gather: return "GATHER";
        case UiDesignationKind::TrafficRestricted: return "TRAFFIC RESTRICTED";
        case UiDesignationKind::UtilityRoute: return "UTILITY ROUTE";
        case UiDesignationKind::EvacuationZone: return "EVACUATION ZONE";
    }
    return "DESIGNATION";
}

const char* tacticalName(UiTacticalOrderKind order) {
    switch(order) {
        case UiTacticalOrderKind::Alert: return "ALERT";
        case UiTacticalOrderKind::Rally: return "RALLY";
        case UiTacticalOrderKind::Patrol: return "PATROL";
        case UiTacticalOrderKind::Breach: return "BREACH";
        case UiTacticalOrderKind::Evacuate: return "EVACUATE";
        case UiTacticalOrderKind::Isolate: return "ISOLATE";
    }
    return "TACTICAL ORDER";
}

} // namespace

const char* uiSeverityName(UiSeverity severity) {
    switch(severity) {
        case UiSeverity::Info: return "INFO";
        case UiSeverity::Advisory: return "ADVISORY";
        case UiSeverity::High: return "HIGH";
        case UiSeverity::Critical: return "CRITICAL";
    }
    return "INFO";
}

std::string formatSurfaceAddress(SurfaceCellAddress address) {
    std::ostringstream out;
    out<<toString(address.face)<<" U"<<address.u<<" V"<<address.v<<" R"<<address.radial;
    return out.str();
}

std::string formatStableId(std::uint64_t stableId) {
    std::ostringstream out;
    out<<"0x"<<std::hex<<std::uppercase<<std::setw(16)<<std::setfill('0')<<stableId;
    return out.str();
}

FortressDashboardModel buildFortressDashboard(const SurfaceInfrastructure& infrastructure,
                                               const UiDiagnosticContext& context) {
    FortressDashboardModel out{};
    out.title="FORTRESS COMMAND // SYSTEM STATUS";

    int machineCount=0;
    int enabledCount=0;
    int atmosphereUnits=0;
    int sealedRooms=0;
    int breathableRooms=0;
    int openAtmosphereUnits=0;
    int logisticsCount=0;
    int productionCount=0;
    int defenseCount=0;
    for(const auto& machine:infrastructure.objects()) {
        ++machineCount;
        if(machine.enabled) ++enabledCount;
        if(machine.type==MachineType::AtmosphereUnit) {
            ++atmosphereUnits;
            if(machine.roomSealed) ++sealedRooms;
            if(machine.roomSealed && machine.roomPressure>=0.55f && machine.roomOxygen>=0.45f) ++breathableRooms;
            if(!machine.roomSealed) ++openAtmosphereUnits;
        }
        if(logisticsMachine(machine.type)) ++logisticsCount;
        if(processMachine(machine.type) || machine.type==MachineType::Extractor) ++productionCount;
        if(machine.type==MachineType::SensorMast || machine.type==MachineType::Turret ||
           machine.type==MachineType::ShieldPylon) ++defenseCount;
    }

    out.overview.push_back({"Machines",std::to_string(machineCount)+" total / "+std::to_string(enabledCount)+" enabled"});
    out.overview.push_back({"Power",floatText(context.power.supplied,1)+" supplied / "+floatText(context.power.demand,1)+" demand"});
    out.overview.push_back({"Generation",floatText(context.power.generation,1)+" // battery "+floatText(context.power.batteryStored,1)+" / "+floatText(context.power.batteryCapacity,1)});
    out.overview.push_back({"Atmosphere",std::to_string(breathableRooms)+" breathable / "+std::to_string(sealedRooms)+" sealed / "+std::to_string(atmosphereUnits)+" units"});
    out.overview.push_back({"Industry",std::to_string(context.industry.activeProcesses)+" active / "+std::to_string(productionCount)+" production machines"});
    out.overview.push_back({"Logistics",std::to_string(logisticsCount)+" movers / "+std::to_string(context.industry.logisticsJams)+" jams"});
    out.overview.push_back({"Defense",std::to_string(context.defense.hostilesDetected)+" contacts / "+std::to_string(defenseCount)+" defense machines"});
    out.overview.push_back({"Empire",std::string(imperialAttentionBandName(context.siege.band))+" / "+registerActionPhaseName(context.siege.phase)});

    const auto alerts=buildInfrastructureAlerts(infrastructure,context);
    out.diagnostics.reserve(alerts.size());
    for(const auto& alert:alerts) {
        out.diagnostics.push_back(UiDiagnostic{alert.key,alert.severity,alert.category,alert.sourceStableId,
                                               alert.title,alert.detail,alert.remediation});
    }

    out.panels={
        {"infrastructure","Infrastructure",UiPanelAvailability::Available,"Machines, doors, airlocks and exact Why diagnostics."},
        {"power","Power",UiPanelAvailability::Available,"Generation, storage, demand and load shedding."},
        {"atmosphere","Atmosphere",UiPanelAvailability::Available,openAtmosphereUnits>0?"One or more atmosphere units currently reference open/unbounded volumes.":"Bounded room pressure and oxygen state."},
        {"industry","Industry",UiPanelAvailability::Available,"Recipes, production blockers and extractor activity."},
        {"logistics","Logistics",UiPanelAvailability::Available,"Network Storage plus explicit conveyors, sorters and cargo loaders."},
        {"defense","Defense",UiPanelAvailability::Available,"Sensors, turrets, shields and Register Action readiness."},
        {"citizens","Citizens",UiPanelAvailability::Unavailable,"Citizen/needs/relationship query APIs are not present in the supplied v0.20 tree."},
        {"jobs","Jobs & Work Orders",UiPanelAvailability::Unavailable,"Autonomous work-order state is not present in the supplied v0.20 tree."},
        {"medicine","Medicine",UiPanelAvailability::Unavailable,"Anatomy/wound/hospital query state is not present in the supplied v0.20 tree."},
        {"justice","Justice",UiPanelAvailability::Unavailable,"Cases/evidence/sentencing state is not present in the supplied v0.20 tree."},
        {"trade","Trade",UiPanelAvailability::Unavailable,"Market/caravan/contract state is not present in the supplied v0.20 tree."},
        {"chronicle","Galactic Chronicle",UiPanelAvailability::Unavailable,"Historical query APIs are not present in the supplied v0.20 tree."}
    };
    return out;
}

WhyInspectorModel buildMachineInspector(const SurfaceInfrastructure& infrastructure,
                                        const SurfaceMachineObject& machine,
                                        const UiDiagnosticContext& context) {
    WhyInspectorModel out{};
    out.kind=UiObjectKind::Machine;
    out.stableId=machine.stableId;
    out.title=machineName(machine.type);
    out.subtitle="WHY // authoritative stable infrastructure state";
    addFact(out,"Stable ID",formatStableId(machine.stableId));
    addFact(out,"Address",formatSurfaceAddress(machine.anchor));
    addFact(out,"Enabled",boolWord(machine.enabled,"yes","no"));
    if(requiresElectricPower(machine.type)) addFact(out,"Power",boolWord(machine.powered,"supplied","not supplied"));
    if(machine.powerNetworkId!=0) addFact(out,"Network",formatStableId(machine.powerNetworkId));
    if(!machine.inventory.empty() || processMachine(machine.type) || logisticsMachine(machine.type) || machine.type==MachineType::NetworkStorage)
        addFact(out,"Inventory",inventoryText(machine));

    diagnosePower(out,machine,context);

    switch(machine.type) {
        case MachineType::BurnerGenerator:
            addFact(out,"Fuel",floatText(machine.fuelSeconds,1)+" s");
            if(machine.enabled && machine.fuelSeconds<=0.0f)
                addDiagnostic(out,"power.generator_no_fuel",UiSeverity::Advisory,"power",machine.stableId,
                              "Burner has no fuel","The generator has no remaining burn time.","Load Coal Ore before local stored energy is exhausted.");
            break;
        case MachineType::BatteryBank:
            addFact(out,"Stored energy",floatText(machine.storedEnergy,1));
            if(machine.storedEnergy<=0.01f && context.power.demand>context.power.generation)
                addDiagnostic(out,"power.battery_empty",UiSeverity::High,"power",machine.stableId,
                              "Battery reserve is empty","Demand exceeds current generation and this bank has no stored energy.","Add generation, charge storage, or shed noncritical loads.");
            break;
        case MachineType::AtmosphereUnit:
            addFact(out,"Room",boolWord(machine.roomSealed,"sealed","open / unbounded"));
            addFact(out,"Pressure",percentText(machine.roomPressure));
            addFact(out,"Oxygen",percentText(machine.roomOxygen));
            if(!machine.roomSealed)
                addDiagnostic(out,"atmosphere.open_volume",machine.roomPressure>0.05f?UiSeverity::Critical:UiSeverity::High,
                              "atmosphere",machine.stableId,
                              machine.roomPressure>0.05f?"Room is decompressing":"Room cannot be pressurized",
                              "The bounded reachable-gas query did not terminate as a sealed volume; a breach/open portal or room-size budget is exposing it.",
                              "Close breaches and portals, then verify the room is within the configured pressurizable-volume budget.");
            else if(machine.roomPressure<0.55f || machine.roomOxygen<0.45f)
                addDiagnostic(out,"atmosphere.not_breathable",UiSeverity::High,"atmosphere",machine.stableId,
                              "Sealed room is not yet breathable",
                              "Pressure or oxygen is below the current breathable threshold.",
                              "Keep the Atmosphere Unit powered while sealed; reduce power pressure if it is being shed.");
            break;
        case MachineType::AirlockController: {
            const auto* airlock=infrastructure.airlockForController(machine.stableId);
            if(!airlock) {
                addDiagnostic(out,"airlock.unpaired",UiSeverity::Advisory,"atmosphere",machine.stableId,
                              "Airlock controller is not paired","No two-door chamber assembly references this controller.",
                              "Pair two Airlock portals and a chamber anchor before relying on interlock protection.");
            } else {
                addFact(out,"Airlock",formatStableId(airlock->stableId));
                addFact(out,"Cycle",surfaceAirlockStateName(airlock->state));
                addFact(out,"Chamber pressure",percentText(airlock->chamberPressure));
                addFact(out,"Chamber oxygen",percentText(airlock->chamberOxygen));
                if(airlock->state==SurfaceAirlockState::Fault)
                    addDiagnostic(out,"airlock.fault",UiSeverity::High,"atmosphere",airlock->stableId,
                                  "Airlock is faulted","The interlock could not continue its requested safe cycle.",
                                  "Restore controller power and inspect both portal records before retrying the cycle.");
            }
            break;
        }
        case MachineType::SensorMast:
            addFact(out,"Detected hostiles",std::to_string(machine.detectedHostiles));
            break;
        case MachineType::Turret:
            addFact(out,"Ammo",std::to_string(machine.ammo)+" rounds");
            addFact(out,"Contacts",std::to_string(machine.detectedHostiles));
            if(machine.detectedHostiles>0 && machine.ammo<=0)
                addDiagnostic(out,"defense.turret_no_ammo",UiSeverity::High,"defense",machine.stableId,
                              "Turret is dry during contact","A hostile is detected but the turret has zero rounds.",
                              "Load ammunition or supply Turret Ammunition through powered same-network Network Storage.");
            break;
        case MachineType::ShieldPylon:
            addFact(out,"Shield charge",floatText(machine.shieldCharge,1));
            if(context.defense.hostilesDetected>0 && machine.powered && machine.shieldCharge<=0.01f)
                addDiagnostic(out,"defense.shield_empty",UiSeverity::High,"defense",machine.stableId,
                              "Shield has no charge during contact","The pylon is powered but its current stored shield charge is empty.",
                              "Keep the pylon powered and reduce incoming pressure until it recharges.");
            break;
        case MachineType::LogicController: {
            int rules=0;
            for(const auto& rule:infrastructure.automationRules()) if(rule.controllerMachineId==machine.stableId) ++rules;
            addFact(out,"Automation rules",std::to_string(rules));
            if(rules==0)
                addDiagnostic(out,"automation.no_rules",UiSeverity::Advisory,"automation",machine.stableId,
                              "Controller has no rules","No automation rule is owned by this controller.",
                              "Create a finite trigger-condition-action rule for a sensor, power, or atmosphere condition.");
            break;
        }
        case MachineType::Furnace:
        case MachineType::AlloyCrucible:
        case MachineType::Refinery:
        case MachineType::Crusher:
        case MachineType::ChemicalVat:
        case MachineType::Fabricator:
        case MachineType::ArcSmelter:
            diagnoseProcessMachine(out,infrastructure,machine);
            break;
        case MachineType::NetworkStorage:
            addFact(out,"Scope",machine.powerNetworkId==0?"not connected":"local base network only");
            break;
        case MachineType::Conveyor:
        case MachineType::Sorter:
        case MachineType::CargoLoader:
            addFact(out,"Source",machine.logisticsSourceStableId?formatStableId(machine.logisticsSourceStableId):"not configured");
            addFact(out,"Target",machine.logisticsTargetStableId?formatStableId(machine.logisticsTargetStableId):"not configured");
            if(machine.type==MachineType::Sorter) {
                addFact(out,"Alternate",machine.logisticsAlternateTargetStableId?formatStableId(machine.logisticsAlternateTargetStableId):"not configured");
                addFact(out,"Filter",machine.sorterFilterItemId?industryItemName(machine.sorterFilterItemId):"not configured");
            }
            if(machine.logisticsSourceStableId==0 || machine.logisticsTargetStableId==0 ||
               (machine.type==MachineType::Sorter && machine.logisticsAlternateTargetStableId==0))
                addDiagnostic(out,"logistics.unconfigured",UiSeverity::Advisory,"logistics",machine.stableId,
                              "Route is incomplete","This transport object does not have every required stable endpoint.",
                              "Assign source and target infrastructure objects; Sorters also require an alternate target and filter.");
            else if(machine.logisticsBlocked)
                addDiagnostic(out,"logistics.jam",UiSeverity::Advisory,"logistics",machine.stableId,
                              "Route is jammed","The current source item has no valid destination with accepting capacity.",
                              "Free target capacity, correct the Sorter filter, or repair the endpoint link.");
            break;
        case MachineType::Extractor:
            addFact(out,"Cycle progress",floatText(machine.extractorProgressSeconds,1)+" s");
            if(machine.powered && !canAcceptLocal(machine,static_cast<int>(BlockType::CoalOre),1) && machine.inventory.size()>=static_cast<std::size_t>(SurfaceIndustrySystem::inventoryCapacity(machine)))
                addDiagnostic(out,"extractor.storage_full",UiSeverity::Advisory,"industry",machine.stableId,
                              "Extractor inventory is saturated","The local machine has no free stack slot; remote output may also be unavailable.",
                              "Free output capacity or connect powered Network Storage in this local base network.");
            break;
        case MachineType::StorageCrate:
            break;
    }

    sortDiagnostics(out.diagnostics);
    return out;
}

WhyInspectorModel buildPortalInspector(const SurfaceInfrastructure& infrastructure,
                                       const SurfacePortalObject& portal) {
    WhyInspectorModel out{};
    out.kind=UiObjectKind::Portal;
    out.stableId=portal.stableId;
    out.title=surfacePortalName(portal.type);
    out.subtitle="WHY // persistent portal state and seal authority";
    addFact(out,"Stable ID",formatStableId(portal.stableId));
    addFact(out,"Address",formatSurfaceAddress(portal.anchor));
    addFact(out,"State",portal.open?"open // gas-passable":"closed // sealing voxel authored");
    if(const auto* assembly=infrastructure.airlockForPortal(portal.stableId)) {
        addFact(out,"Interlock",formatStableId(assembly->stableId));
        addFact(out,"Cycle",surfaceAirlockStateName(assembly->state));
        if(portal.open)
            addDiagnostic(out,"airlock.portal_open",UiSeverity::Info,"atmosphere",portal.stableId,
                          "Portal is open under interlock authority","A powered airlock cycle currently owns this portal state.",
                          "Use the airlock controller rather than bypassing the portal directly.");
    } else if(portal.type==SurfacePortalType::Airlock) {
        addDiagnostic(out,"airlock.portal_unpaired",UiSeverity::Advisory,"atmosphere",portal.stableId,
                      "Airlock portal is not interlocked","No SurfaceAirlockAssembly references this portal.",
                      "Pair it with a second Airlock portal and an Airlock Controller for controlled pressure transitions.");
    }
    sortDiagnostics(out.diagnostics);
    return out;
}

WhyInspectorModel buildAirlockInspector(const SurfaceInfrastructure& infrastructure,
                                        const SurfaceAirlockAssembly& airlock) {
    WhyInspectorModel out{};
    out.kind=UiObjectKind::Airlock;
    out.stableId=airlock.stableId;
    out.title="Airlock Assembly";
    out.subtitle="WHY // two-door pressure transition";
    addFact(out,"Stable ID",formatStableId(airlock.stableId));
    addFact(out,"Controller",formatStableId(airlock.controllerMachineId));
    addFact(out,"Inner portal",formatStableId(airlock.innerPortalId));
    addFact(out,"Outer portal",formatStableId(airlock.outerPortalId));
    addFact(out,"Chamber",formatSurfaceAddress(airlock.chamberAnchor));
    addFact(out,"Cycle",surfaceAirlockStateName(airlock.state));
    addFact(out,"Pressure",percentText(airlock.chamberPressure));
    addFact(out,"Oxygen",percentText(airlock.chamberOxygen));
    if(!infrastructure.find(airlock.controllerMachineId))
        addDiagnostic(out,"airlock.controller_missing",UiSeverity::High,"atmosphere",airlock.stableId,
                      "Airlock controller is missing","The assembly references a controller StableId not present in the active infrastructure set.",
                      "Restore/load the dependency or repair the assembly record before use.");
    if(!infrastructure.findPortal(airlock.innerPortalId) || !infrastructure.findPortal(airlock.outerPortalId))
        addDiagnostic(out,"airlock.portal_missing",UiSeverity::High,"atmosphere",airlock.stableId,
                      "Airlock portal dependency is missing","One or both stable portal dependencies are unresolved.",
                      "Restore/load the portal records or repair the assembly before cycling.");
    if(airlock.state==SurfaceAirlockState::Fault)
        addDiagnostic(out,"airlock.fault",UiSeverity::High,"atmosphere",airlock.stableId,
                      "Airlock is faulted","The current cycle cannot proceed safely.","Restore power/dependencies, close both doors, and retry.");
    sortDiagnostics(out.diagnostics);
    return out;
}

std::vector<UiAlert> buildInfrastructureAlerts(const SurfaceInfrastructure& infrastructure,
                                               const UiDiagnosticContext& context) {
    std::map<std::string,UiAlert,std::less<>> alerts;

    if(context.power.shedLoads>0) {
        addAlert(alerts,{"power.brownout",UiSeverity::High,"power",0,"BASE BROWNOUT",
                         std::to_string(context.power.shedLoads)+" load(s) are shed; supplied "+floatText(context.power.supplied,0)+
                         " / "+floatText(context.power.demand,0)+" demand.",
                         "Add generation/storage or disable lower-value loads. Life support should remain the highest priority."});
    }

    for(const auto& machine:infrastructure.objects()) {
        if(machine.type==MachineType::AtmosphereUnit && machine.enabled) {
            if(!machine.roomSealed && machine.roomPressure>0.05f) {
                addAlert(alerts,{"atmosphere.decompression."+formatStableId(machine.stableId),UiSeverity::Critical,"atmosphere",machine.stableId,
                                 "DECOMPRESSION",
                                 std::string(machineName(machine.type))+" at "+formatSurfaceAddress(machine.anchor)+" is venting from "+percentText(machine.roomPressure)+" pressure.",
                                 "Close the breach/portal and keep life support powered; verify the room fits inside the pressure-volume budget."});
            } else if(machine.roomSealed && (machine.roomPressure<0.55f || machine.roomOxygen<0.45f)) {
                addAlert(alerts,{"atmosphere.low_life_support."+formatStableId(machine.stableId),UiSeverity::High,"atmosphere",machine.stableId,
                                 "LOW ROOM LIFE SUPPORT",
                                 "Sealed room is "+percentText(machine.roomPressure)+" pressure / "+percentText(machine.roomOxygen)+" O2.",
                                 "Keep the Atmosphere Unit powered and reduce base power pressure until the room becomes breathable."});
            }
        }
        if(machine.type==MachineType::Turret && machine.enabled && machine.detectedHostiles>0 && machine.ammo<=0) {
            addAlert(alerts,{"defense.no_ammo."+formatStableId(machine.stableId),UiSeverity::High,"defense",machine.stableId,
                             "DEFENSE AMMO EMPTY",
                             "Turret at "+formatSurfaceAddress(machine.anchor)+" sees "+std::to_string(machine.detectedHostiles)+" contact(s) with zero rounds.",
                             "Load ammunition or route fabricated Turret Ammunition through powered same-network Network Storage."});
        }
    }

    if(context.defense.hostilesDetected>0) {
        addAlert(alerts,{"defense.active_contact",UiSeverity::Critical,"defense",0,"ACTIVE HOSTILE CONTACT",
                         std::to_string(context.defense.hostilesDetected)+" hostile contact(s) are inside powered sensor coverage.",
                         "Check turret ammo, shield charge, doors, and the current Register Action before committing to exterior work."});
    }

    if(context.siege.phase==RegisterActionPhase::Announced) {
        addAlert(alerts,{"empire.register_announced",UiSeverity::High,"empire",context.siege.actionId,
                         "REGISTER ACTION ANNOUNCED",
                         std::string(imperialAttentionBandName(context.siege.band))+" enforcement begins in "+floatText(context.siege.phaseSecondsRemaining,0)+" s.",
                         "Stage ammunition and power, close vulnerable portals, and protect the Registry Beacon."});
    } else if(context.siege.phase==RegisterActionPhase::WaveActive || context.siege.phase==RegisterActionPhase::InterWave) {
        addAlert(alerts,{"empire.register_active",UiSeverity::Critical,"empire",context.siege.actionId,
                         "REGISTER ACTION ACTIVE",
                         "Wave "+std::to_string(context.siege.waveIndex+1)+" / "+std::to_string(context.siege.totalWaves)+" is active or regrouping.",
                         "Protect the Registry Beacon and use local sensors, turrets, shields, and access controls to contain the attack."});
    }

    if(context.industry.logisticsJams>0) {
        addAlert(alerts,{"logistics.jam",UiSeverity::Advisory,"logistics",0,"LOGISTICS JAM",
                         std::to_string(context.industry.logisticsJams)+" transport attempt(s) were blocked this update.",
                         "Inspect the affected Conveyor/Sorter/Cargo Loader for target capacity, filter, and stable endpoint links."});
    }
    if(context.industry.stalledForOutput>0) {
        addAlert(alerts,{"industry.output_blocked",UiSeverity::Advisory,"industry",0,"PRODUCTION OUTPUT BLOCKED",
                         std::to_string(context.industry.stalledForOutput)+" process machine(s) could not publish output.",
                         "Free machine inventory or powered same-network Network Storage capacity."});
    }

    std::vector<UiAlert> out;
    out.reserve(alerts.size());
    for(auto& [key,alert]:alerts) { (void)key; out.push_back(std::move(alert)); }
    std::stable_sort(out.begin(),out.end(),[](const auto& a,const auto& b){
        if(severityRank(a.severity)!=severityRank(b.severity)) return severityRank(a.severity)>severityRank(b.severity);
        if(a.category!=b.category) return a.category<b.category;
        if(a.sourceStableId!=b.sourceStableId) return a.sourceStableId<b.sourceStableId;
        return a.key<b.key;
    });
    return out;
}

std::vector<UiPresentedAlert> UiAlertCenter::update(const std::vector<UiAlert>& candidates) {
    for(auto& [key,state]:states_) { (void)key; state.active=false; }
    std::map<std::string,UiAlert,std::less<>> unique;
    for(const auto& candidate:candidates) {
        auto [it,inserted]=unique.emplace(candidate.key,candidate);
        if(!inserted && severityRank(candidate.severity)>severityRank(it->second.severity)) it->second=candidate;
    }

    std::vector<UiPresentedAlert> out;
    for(const auto& [key,alert]:unique) {
        auto& state=states_[key];
        const std::string fingerprint=alertFingerprint(alert);
        if(state.fingerprint!=fingerprint) {
            state.fingerprint=fingerprint;
            state.acknowledged=false;
        }
        state.active=true;
        const bool muted=categoryMuted(alert.category);
        if(state.pinned || (!muted && !state.acknowledged)) out.push_back({alert,state.acknowledged,state.pinned});
    }

    for(auto it=states_.begin();it!=states_.end();) {
        if(!it->second.active && !it->second.pinned) it=states_.erase(it);
        else ++it;
    }
    std::stable_sort(out.begin(),out.end(),[](const auto& a,const auto& b){
        if(a.pinned!=b.pinned) return a.pinned>b.pinned;
        if(severityRank(a.alert.severity)!=severityRank(b.alert.severity)) return severityRank(a.alert.severity)>severityRank(b.alert.severity);
        return a.alert.key<b.alert.key;
    });
    return out;
}

bool UiAlertCenter::acknowledge(std::string_view key) {
    const auto it=states_.find(key);
    if(it==states_.end()) return false;
    it->second.acknowledged=true;
    return true;
}

bool UiAlertCenter::setPinned(std::string_view key, bool pinned) {
    const auto it=states_.find(key);
    if(it==states_.end()) return false;
    it->second.pinned=pinned;
    return true;
}

void UiAlertCenter::setCategoryMuted(std::string category, bool muted) {
    if(category.empty()) return;
    if(muted) mutedCategories_.insert(std::move(category));
    else mutedCategories_.erase(category);
}

bool UiAlertCenter::categoryMuted(std::string_view category) const {
    return mutedCategories_.find(category)!=mutedCategories_.end();
}

void UiAlertCenter::clearPresentationState() {
    states_.clear();
    mutedCategories_.clear();
}

UiCommandRequest makeSetMachineEnabledCommand(std::uint64_t machineStableId, bool enabled) {
    UiCommandRequest out{};
    out.kind=UiCommandKind::SetMachineEnabled;
    out.primaryStableId=machineStableId;
    out.boolValue=enabled;
    return out;
}

UiCommandRequest makeSetPortalOpenCommand(std::uint64_t portalStableId, bool open) {
    UiCommandRequest out{};
    out.kind=UiCommandKind::SetPortalOpen;
    out.primaryStableId=portalStableId;
    out.boolValue=open;
    return out;
}

UiCommandRequest makeSelectRecipeCommand(std::uint64_t machineStableId, SurfaceRecipeId recipe) {
    UiCommandRequest out{};
    out.kind=UiCommandKind::SelectRecipe;
    out.primaryStableId=machineStableId;
    out.intValue=static_cast<int>(recipe);
    return out;
}

UiCommandRequest makeConfigureLogisticsCommand(std::uint64_t transportStableId,
                                               std::uint64_t sourceStableId,
                                               std::uint64_t targetStableId,
                                               std::uint64_t alternateTargetStableId,
                                               int sorterFilterItemId) {
    UiCommandRequest out{};
    out.kind=UiCommandKind::ConfigureLogistics;
    out.primaryStableId=transportStableId;
    out.secondaryStableId=sourceStableId;
    out.tertiaryStableId=targetStableId;
    out.alternateStableId=alternateTargetStableId;
    out.intValue=sorterFilterItemId;
    return out;
}

UiCommandRequest makeClearLogisticsCommand(std::uint64_t transportStableId) {
    UiCommandRequest out{};
    out.kind=UiCommandKind::ClearLogistics;
    out.primaryStableId=transportStableId;
    return out;
}

UiCommandRequest makeDesignationCommand(UiDesignationKind designation,
                                        SurfaceCellAddress cell,
                                        int priority) {
    UiCommandRequest out{};
    out.kind=UiCommandKind::Designation;
    out.designation=designation;
    out.cell=cell;
    out.hasCell=true;
    out.intValue=priority;
    return out;
}

UiCommandRequest makeTacticalOrderCommand(UiTacticalOrderKind order,
                                          SurfaceCellAddress cell,
                                          std::uint64_t squadStableId) {
    UiCommandRequest out{};
    out.kind=UiCommandKind::TacticalOrder;
    out.tacticalOrder=order;
    out.cell=cell;
    out.hasCell=true;
    out.primaryStableId=squadStableId;
    return out;
}

bool validateUiCommand(const UiCommandRequest& command, std::string* error) {
    auto fail=[&](const char* message){if(error) *error=message; return false;};
    switch(command.kind) {
        case UiCommandKind::SetMachineEnabled:
            if(command.primaryStableId==0) return fail("machine StableId is required");
            break;
        case UiCommandKind::SetPortalOpen:
            if(command.primaryStableId==0) return fail("portal StableId is required");
            break;
        case UiCommandKind::SelectRecipe:
            if(command.primaryStableId==0) return fail("machine StableId is required");
            if(command.intValue<=0 || !surfaceRecipe(static_cast<SurfaceRecipeId>(command.intValue))) return fail("valid nonzero recipe is required");
            break;
        case UiCommandKind::ConfigureLogistics:
            if(command.primaryStableId==0 || command.secondaryStableId==0 || command.tertiaryStableId==0)
                return fail("transport, source, and target StableIds are required");
            if(command.primaryStableId==command.secondaryStableId || command.primaryStableId==command.tertiaryStableId)
                return fail("transport cannot be its own source or target");
            if(command.secondaryStableId==command.tertiaryStableId)
                return fail("source and target must differ");
            break;
        case UiCommandKind::ClearLogistics:
            if(command.primaryStableId==0) return fail("transport StableId is required");
            break;
        case UiCommandKind::Designation:
            if(!command.hasCell) return fail("designation requires a surface address");
            if(command.intValue<0 || command.intValue>9) return fail("designation priority must be in 0..9");
            break;
        case UiCommandKind::TacticalOrder:
            if(!command.hasCell) return fail("tactical order requires a surface address");
            break;
    }
    if(error) error->clear();
    return true;
}

std::string formatUiCommand(const UiCommandRequest& command) {
    std::ostringstream out;
    switch(command.kind) {
        case UiCommandKind::SetMachineEnabled:
            out<<(command.boolValue?"ENABLE MACHINE ":"DISABLE MACHINE ")<<formatStableId(command.primaryStableId);
            break;
        case UiCommandKind::SetPortalOpen:
            out<<(command.boolValue?"OPEN PORTAL ":"CLOSE PORTAL ")<<formatStableId(command.primaryStableId);
            break;
        case UiCommandKind::SelectRecipe: {
            const auto* recipe=surfaceRecipe(static_cast<SurfaceRecipeId>(command.intValue));
            out<<"SELECT RECIPE "<<(recipe?recipe->name:std::string_view{"INVALID"})<<" ON "<<formatStableId(command.primaryStableId);
            break;
        }
        case UiCommandKind::ConfigureLogistics:
            out<<"LINK "<<formatStableId(command.primaryStableId)<<" : "<<formatStableId(command.secondaryStableId)
               <<" -> "<<formatStableId(command.tertiaryStableId);
            if(command.alternateStableId) out<<" ALT "<<formatStableId(command.alternateStableId);
            if(command.intValue) out<<" FILTER "<<industryItemName(command.intValue);
            break;
        case UiCommandKind::ClearLogistics:
            out<<"CLEAR LOGISTICS "<<formatStableId(command.primaryStableId);
            break;
        case UiCommandKind::Designation:
            out<<designationName(command.designation)<<" @ "<<formatSurfaceAddress(command.cell)<<" PRIORITY "<<command.intValue;
            break;
        case UiCommandKind::TacticalOrder:
            out<<tacticalName(command.tacticalOrder)<<" @ "<<formatSurfaceAddress(command.cell);
            if(command.primaryStableId) out<<" SQUAD "<<formatStableId(command.primaryStableId);
            break;
    }
    return out.str();
}

} // namespace elysium
