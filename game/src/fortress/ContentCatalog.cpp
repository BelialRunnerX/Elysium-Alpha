#include "fortress/ContentCatalog.hpp"

#include <algorithm>
#include <array>

namespace elysium::fortress {
namespace {

constexpr std::array kLabors{
    LaborDefinition{"elysium:labor/mining", "excavation", "Mine macrovoxel / vein", "tool tier + path + designation"},
    LaborDefinition{"elysium:labor/precision_mining", "excavation", "Micro-sculpt / controlled breach", "fine tool + micro-refinable material"},
    LaborDefinition{"elysium:labor/channeling", "excavation", "Excavate channel", "support + route"},
    LaborDefinition{"elysium:labor/shaft_work", "excavation", "Build stairs / ramps", "mining + construction skill"},
    LaborDefinition{"elysium:labor/geology", "excavation", "Survey strata / vein", "scanner or geology knowledge"},
    LaborDefinition{"elysium:labor/demolition", "excavation", "Remove construction", "authorization + tool policy"},
    LaborDefinition{"elysium:labor/salvage_cutting", "excavation", "Dismantle wreck / ruin", "tool + environment"},
    LaborDefinition{"elysium:labor/masonry", "construction", "Build stone structure", "stone + plan"},
    LaborDefinition{"elysium:labor/carpentry", "construction", "Build timber components", "timber + plan"},
    LaborDefinition{"elysium:labor/metal_construction", "construction", "Build metal structure", "plate + beams + tools"},
    LaborDefinition{"elysium:labor/concrete", "construction", "Pour/set composite", "mix + environment"},
    LaborDefinition{"elysium:labor/micro_detail", "construction", "Trim / repair / sculpt", "fine material + skill"},
    LaborDefinition{"elysium:labor/reinforcement", "construction", "Upgrade structure", "support + compatible material"},
    LaborDefinition{"elysium:labor/sealing", "construction", "Apply pressure treatment", "sealant + clean surface"},
    LaborDefinition{"elysium:labor/insulation", "construction", "Install insulation", "insulation material"},
    LaborDefinition{"elysium:labor/glazing", "construction", "Install windows", "glass + frame"},
    LaborDefinition{"elysium:labor/furniture_install", "construction", "Install furniture", "item + room plan"},
    LaborDefinition{"elysium:labor/utility_install", "construction", "Pipe/duct/cable/rail", "component + route"},
    LaborDefinition{"elysium:labor/machine_install", "construction", "Place/commission machine", "foundation + components + power test"},
    LaborDefinition{"elysium:labor/repair_structure", "construction", "Repair building", "material + access"},
    LaborDefinition{"elysium:labor/deconstruct", "construction", "Recover components", "designation + hauling"},
    LaborDefinition{"elysium:labor/haul_general", "hauling", "Move item to stockpile", "accepted destination"},
    LaborDefinition{"elysium:labor/haul_food", "hauling", "Move perishables", "food-safe destination"},
    LaborDefinition{"elysium:labor/haul_refuse", "hauling", "Move waste/corpses", "hazard policy"},
    LaborDefinition{"elysium:labor/haul_container", "hauling", "Move crate/bin/tank", "capacity + path"},
    LaborDefinition{"elysium:labor/haul_construction", "hauling", "Stage materials", "job reservation"},
    LaborDefinition{"elysium:labor/haul_workshop", "hauling", "Deliver/remove process items", "port reservation"},
    LaborDefinition{"elysium:labor/haul_medical", "hauling", "Stock hospital", "hospital request"},
    LaborDefinition{"elysium:labor/haul_military", "hauling", "Move ammo/equipment", "squad policy"},
    LaborDefinition{"elysium:labor/haul_trade", "hauling", "Move trade cargo", "manifest + depot"},
    LaborDefinition{"elysium:labor/vehicle_loading", "hauling", "Load/unload vehicle", "dock + manifest"},
    LaborDefinition{"elysium:labor/artifact_transport", "hauling", "Move protected artifact", "security + ownership"},
    LaborDefinition{"elysium:labor/hazmat_haul", "hauling", "Move contaminated material", "PPE + hazard stockpile"},
    LaborDefinition{"elysium:labor/stonecutting", "stone_wood", "Cut blocks/slabs", "stone + workshop"},
    LaborDefinition{"elysium:labor/stone_carving", "stone_wood", "Carve furniture/decor", "skill + workshop"},
    LaborDefinition{"elysium:labor/engraving", "stone_wood", "Engrave surface", "finished surface + motif"},
    LaborDefinition{"elysium:labor/gem_cutting", "stone_wood", "Cut gems/crystals", "precision tool"},
    LaborDefinition{"elysium:labor/wood_cutting", "stone_wood", "Harvest organics", "designation + tool"},
    LaborDefinition{"elysium:labor/fiber_work", "stone_wood", "Spin/weave fiber", "fiber + textile bench"},
    LaborDefinition{"elysium:labor/smelting", "metallurgy", "Ore to metal", "furnace + fuel/power"},
    LaborDefinition{"elysium:labor/alloying", "metallurgy", "Combine metals", "recipe + crucible"},
    LaborDefinition{"elysium:labor/blacksmithing", "metallurgy", "Forge tools/weapons", "metal + forge"},
    LaborDefinition{"elysium:labor/armorsmithing", "metallurgy", "Armor/suit plates", "forge/fabricator"},
    LaborDefinition{"elysium:labor/machining", "metallurgy", "Precision metal parts", "machine shop"},
    LaborDefinition{"elysium:labor/casting", "metallurgy", "Cast parts", "molten material + mold"},
    LaborDefinition{"elysium:labor/electronics", "fabrication", "Circuits/sensors", "clean powered bench"},
    LaborDefinition{"elysium:labor/fabrication", "fabrication", "Modules/components", "fabricator + recipe"},
    LaborDefinition{"elysium:labor/optics", "fabrication", "Scanners/windows/optics", "clean bench"},
    LaborDefinition{"elysium:labor/exotic_metallurgy", "fabrication", "Voidglass/Aetherium/Neutronium", "arc smelter + specialist"},
    LaborDefinition{"elysium:labor/power_engineering", "utilities", "Operate/repair power", "network access"},
    LaborDefinition{"elysium:labor/atmosphere", "utilities", "Life support service", "duct/room access"},
    LaborDefinition{"elysium:labor/pumping", "utilities", "Fluid handling", "pipe/pump access"},
    LaborDefinition{"elysium:labor/refrigeration", "utilities", "Climate service", "power + coolant"},
    LaborDefinition{"elysium:labor/reactor_operation", "utilities", "Operate reactor", "clearance + training"},
    LaborDefinition{"elysium:labor/mechanics", "utilities", "Repair machinery", "tools + parts"},
    LaborDefinition{"elysium:labor/farming", "agriculture", "Plant/harvest crops", "farm access"},
    LaborDefinition{"elysium:labor/hydroponics", "agriculture", "Run hydroponics", "water/nutrients/light"},
    LaborDefinition{"elysium:labor/animal_handling", "agriculture", "Feed/train livestock", "animal access"},
    LaborDefinition{"elysium:labor/butchery", "agriculture", "Process carcass", "clean workshop"},
    LaborDefinition{"elysium:labor/cooking", "agriculture", "Prepare meals", "kitchen + ingredients"},
    LaborDefinition{"elysium:labor/fermentation", "agriculture", "Brew/culture food", "containers + temperature"},
    LaborDefinition{"elysium:labor/diagnosis", "medicine", "Diagnose patient", "hospital + knowledge"},
    LaborDefinition{"elysium:labor/trauma_care", "medicine", "Stabilize injury", "medical supplies"},
    LaborDefinition{"elysium:labor/surgery", "medicine", "Surgical treatment", "clean room + tools"},
    LaborDefinition{"elysium:labor/bone_setting", "medicine", "Set fracture", "splint + skill"},
    LaborDefinition{"elysium:labor/pharmacology", "medicine", "Synthesize/administer medicine", "samples + clean lab"},
    LaborDefinition{"elysium:labor/rehabilitation", "medicine", "Restore function", "patient + rehab space"},
    LaborDefinition{"elysium:labor/prosthetics", "medicine", "Fit prosthetic", "device + precision shop"},
    LaborDefinition{"elysium:labor/research", "knowledge", "Research project", "archive/lab + samples"},
    LaborDefinition{"elysium:labor/xenobiology", "knowledge", "Study life", "specimens + lab"},
    LaborDefinition{"elysium:labor/rift_science", "knowledge", "Study anomalies", "shielded lab"},
    LaborDefinition{"elysium:labor/writing", "knowledge", "Author work", "time + knowledge"},
    LaborDefinition{"elysium:labor/teaching", "knowledge", "Teach skill", "student + institution"},
    LaborDefinition{"elysium:labor/cartography", "knowledge", "Map world/system", "survey data"},
    LaborDefinition{"elysium:labor/appraisal", "knowledge", "Value goods/artifacts", "market/archive"},
    LaborDefinition{"elysium:labor/bookkeeping", "administration", "Inventory/records", "office"},
    LaborDefinition{"elysium:labor/brokerage", "administration", "Negotiate trade", "market + mandate"},
    LaborDefinition{"elysium:labor/leadership", "administration", "Command/organize", "office/squad"},
    LaborDefinition{"elysium:labor/investigation", "administration", "Investigate case", "evidence + jurisdiction"},
    LaborDefinition{"elysium:labor/law", "administration", "Adjudicate case", "office + evidence"},
    LaborDefinition{"elysium:labor/persuasion", "administration", "Social negotiation", "target access"},
    LaborDefinition{"elysium:labor/performance", "culture", "Perform art/ritual", "institution + audience"},
    LaborDefinition{"elysium:labor/security", "combat", "Patrol/guard/respond", "weapon + schedule"},
    LaborDefinition{"elysium:labor/tactics", "combat", "Plan squad action", "squad + command"},
    LaborDefinition{"elysium:labor/heavy_weapons", "combat", "Operate heavy weapon", "ammo + training"},
    LaborDefinition{"elysium:labor/piloting", "combat", "Pilot vehicle/ship", "vehicle + clearance"},
    LaborDefinition{"elysium:labor/fire_response", "emergency", "Suppress fire / manage smoke", "alarm + suppression gear + safe route"},
    LaborDefinition{"elysium:labor/decontamination", "emergency", "Decontaminate room/item/body", "PPE + cleaning/decon supplies"},
    LaborDefinition{"elysium:labor/evacuation_response", "emergency", "Guide evacuation / seal unsafe district", "alert route + safe zone"},
    LaborDefinition{"elysium:labor/emergency_utility", "emergency", "Secure critical utility during incident", "utility access + hazard protection"},
    LaborDefinition{"elysium:labor/rescue", "emergency", "Recover trapped/injured person", "route + carry capacity + protection"},
    LaborDefinition{"elysium:labor/quarantine", "medicine", "Move/isolate infectious patient", "quarantine room + medical clearance"},
    LaborDefinition{"elysium:labor/long_term_care", "medicine", "Provide continuing patient care", "hospital bed + supplies + schedule"},
    LaborDefinition{"elysium:labor/muster", "military", "Assemble squad at rally zone", "squad assignment + alert"},
    LaborDefinition{"elysium:labor/drill", "military", "Train squad tactics and discipline", "barracks/academy + schedule"},
    LaborDefinition{"elysium:labor/patrol", "military", "Patrol assigned route", "squad/guard assignment + access"},
    LaborDefinition{"elysium:labor/prisoner_escort", "military", "Escort detained person", "custody order + secure destination"},
    LaborDefinition{"elysium:labor/counterintelligence", "security", "Investigate infiltration indicators", "security data + clearance"},
    LaborDefinition{"elysium:labor/interrogation", "security", "Question suspect/witness", "case + lawful authority"},
    LaborDefinition{"elysium:labor/civic_participation", "civic", "Attend public institution activity", "available location + schedule"},
    LaborDefinition{"elysium:labor/guild_demonstration", "civic", "Teach public guild demonstration", "guildhall + skill + audience"},
    LaborDefinition{"elysium:labor/petition_review", "civic", "Review petition / residency request", "office/forum + authority"},
    LaborDefinition{"elysium:labor/memorial_administration", "civic", "Manage memorial/burial record", "remains/history + memorial space"},
    LaborDefinition{"elysium:labor/artifact_craft", "culture", "Create Aetheric masterwork", "exclusive workshop + demanded materials"},
    LaborDefinition{"elysium:labor/drone_operation", "logistics", "Service/dispatch autonomous drones", "drone port + power + parts"},
    LaborDefinition{"elysium:labor/rail_operation", "logistics", "Operate/maintain cargo rail", "rail route + controls + brakes"},
    LaborDefinition{"elysium:labor/inspection", "administration", "Inspect cargo/site/records", "jurisdiction + sensors/manifest"}
};

constexpr std::array kMachines{
    MachineDefinition{"elysium:machine/workbench", "survival", "general fabrication", "operator", "none", 1},
    MachineDefinition{"elysium:machine/furnace", "survival", "smelt/glass/cooking variants", "operator", "fuel/power + ventilation", 1},
    MachineDefinition{"elysium:machine/kitchen", "survival", "meals/preservation", "operator", "clean food area", 1},
    MachineDefinition{"elysium:machine/fermenter", "survival", "drinks/cultures", "operator", "containers + temperature", 1},
    MachineDefinition{"elysium:machine/textile_bench", "survival", "fiber/cloth", "operator", "dry storage", 1},
    MachineDefinition{"elysium:machine/alloy_crucible", "metallurgy", "bronze/brass/special alloys", "operator", "heat", 2},
    MachineDefinition{"elysium:machine/blast_furnace", "metallurgy", "steel/tier-4 metallurgy", "operator", "power/heat/ventilation", 3},
    MachineDefinition{"elysium:machine/foundry", "metallurgy", "cast parts", "operator", "molten handling", 3},
    MachineDefinition{"elysium:machine/machine_shop", "metallurgy", "precision metal parts", "operator", "power", 3},
    MachineDefinition{"elysium:machine/arc_smelter", "metallurgy", "Voidglass/Aetherium/Neutronium", "operator", "high power/shielding", 4},
    MachineDefinition{"elysium:machine/electronics_bench", "fabrication", "circuits/sensors", "operator", "clean/power", 3},
    MachineDefinition{"elysium:machine/fabricator", "fabrication", "precision modules", "operator/automated", "power", 4},
    MachineDefinition{"elysium:machine/composite_press", "fabrication", "panels/armor composites", "operator/automated", "resin/pressure", 4},
    MachineDefinition{"elysium:machine/optics_bench", "fabrication", "scanners/windows/data optics", "operator", "clean environment", 3},
    MachineDefinition{"elysium:machine/elysium_workstation", "fabrication", "runes/reforge/ascension", "operator", "exotic power/material", 4},
    MachineDefinition{"elysium:machine/crusher", "chemical", "ore/stone preprocessing", "automated/operator", "power", 2},
    MachineDefinition{"elysium:machine/chemical_vat", "chemical", "resins/acids/sealants/catalysts", "operator/automated", "vent/drain", 3},
    MachineDefinition{"elysium:machine/separator", "chemical", "chemical/biological separation", "automated", "power", 3},
    MachineDefinition{"elysium:machine/medical_synthesizer", "chemical", "gel/drugs/antidotes", "operator", "clean samples", 3},
    MachineDefinition{"elysium:machine/nutrient_mixer", "chemical", "hydroponic solutions", "automated", "water/minerals", 3},
    MachineDefinition{"elysium:machine/burner_generator", "power", "early power", "automated", "fuel", 2},
    MachineDefinition{"elysium:machine/solar_collector", "power", "daylight power", "automated", "exposure", 3},
    MachineDefinition{"elysium:machine/wind_harvester", "power", "weather power", "automated", "exposure", 3},
    MachineDefinition{"elysium:machine/thermal_collector", "power", "thermal-gradient power", "automated", "gradient", 3},
    MachineDefinition{"elysium:machine/uranium_reactor", "power", "high continuous power", "operator/automated", "cooling/shielding", 4},
    MachineDefinition{"elysium:machine/aetheric_converter", "power", "endgame exotic power", "specialist", "anomaly stability", 5},
    MachineDefinition{"elysium:machine/battery_bank", "power", "energy storage", "automated", "network", 2},
    MachineDefinition{"elysium:machine/capacitor_array", "power", "high discharge defense buffer", "automated", "network", 4},
    MachineDefinition{"elysium:machine/atmosphere_unit", "habitat", "pressurize/filter rooms", "automated", "power/ducts", 2},
    MachineDefinition{"elysium:machine/water_recycler", "habitat", "purify/recycle water", "automated", "power/pipes", 2},
    MachineDefinition{"elysium:machine/climate_unit", "habitat", "heat/cool/humidity", "automated", "power/ducts", 3},
    MachineDefinition{"elysium:machine/hydroponic_bed", "habitat", "controlled crops", "farmer/automated", "light/water/nutrients", 3},
    MachineDefinition{"elysium:machine/cryo_storage", "habitat", "preserve food/samples", "automated", "power", 3},
    MachineDefinition{"elysium:machine/med_bay", "habitat", "diagnostic/treatment support", "doctor", "power/supplies", 3},
    MachineDefinition{"elysium:machine/decon_unit", "habitat", "clean actors/items", "automated", "power/water/drain", 3},
    MachineDefinition{"elysium:machine/storage_crate", "logistics", "local storage", "none", "none", 1},
    MachineDefinition{"elysium:machine/network_storage", "logistics", "shared inventory", "automated", "power", 3},
    MachineDefinition{"elysium:machine/conveyor", "logistics", "item flow", "automated", "power optional", 3},
    MachineDefinition{"elysium:machine/sorter", "logistics", "filtered routing", "automated", "power/control", 3},
    MachineDefinition{"elysium:machine/pipe_pump", "logistics", "fluid transfer", "automated", "power", 3},
    MachineDefinition{"elysium:machine/cargo_loader", "logistics", "vehicle/ship transfer", "automated/operator", "power/dock", 3},
    MachineDefinition{"elysium:machine/drone_port", "logistics", "hauling/repair drones", "automated", "power/parts", 4},
    MachineDefinition{"elysium:machine/cargo_rail_stop", "logistics", "bulk freight routing", "automated", "power/rail", 4},
    MachineDefinition{"elysium:machine/turret", "defense", "automated defense", "automated", "power/ammo", 3},
    MachineDefinition{"elysium:machine/shield_pylon", "defense", "shield coverage", "automated", "high power", 4},
    MachineDefinition{"elysium:machine/sensor_mast", "defense", "detection/forecasting", "automated", "power/exposure", 2},
    MachineDefinition{"elysium:machine/decoy_beacon", "defense", "redirect dispatches", "automated", "power", 4},
    MachineDefinition{"elysium:machine/blast_door", "defense", "hardened seal", "automated", "power", 3},
    MachineDefinition{"elysium:machine/interdictor", "defense", "phase denial", "automated", "exotic power", 5},
    MachineDefinition{"elysium:machine/registry_beacon", "command", "claim anchor", "none", "power", 3},
    MachineDefinition{"elysium:machine/logic_controller", "command", "automation rules", "automated", "power", 3},
    MachineDefinition{"elysium:machine/research_console", "command", "research interface", "researcher", "power", 3},
    MachineDefinition{"elysium:machine/map_room", "command", "planning/survey", "operator", "power", 3}
};

constexpr std::array kInstitutions{
    InstitutionDefinition{"elysium:institution/cantina", "food, drink, socialization and performance"},
    InstitutionDefinition{"elysium:institution/shrine", "faith/philosophy and ritual"},
    InstitutionDefinition{"elysium:institution/guildhall", "profession, mentorship and petitions"},
    InstitutionDefinition{"elysium:institution/archive", "knowledge, history and research"},
    InstitutionDefinition{"elysium:institution/hospital", "medicine, quarantine and rehabilitation"},
    InstitutionDefinition{"elysium:institution/memorial", "remembrance, grief and civic identity"},
    InstitutionDefinition{"elysium:institution/academy", "teaching, drills and scholarship"},
    InstitutionDefinition{"elysium:institution/forum", "governance, petitions and public events"}
};

constexpr std::array kThreats{
    ThreatDefinition{"elysium:threat/wildlife", ThreatKind::Wildlife, 0.5f, false},
    ThreatDefinition{"elysium:threat/raider", ThreatKind::Raider, 1.0f, true},
    ThreatDefinition{"elysium:threat/register_action", ThreatKind::ImperialRegisterAction, 2.0f, true},
    ThreatDefinition{"elysium:threat/infiltration", ThreatKind::Infiltration, 0.8f, true},
    ThreatDefinition{"elysium:threat/rift_horror", ThreatKind::RiftHorror, 4.0f, true},
    ThreatDefinition{"elysium:threat/titan", ThreatKind::Titan, 8.0f, true},
    ThreatDefinition{"elysium:threat/plague", ThreatKind::Plague, 2.0f, true},
    ThreatDefinition{"elysium:threat/reactor_incident", ThreatKind::ReactorIncident, 3.0f, true},
    ThreatDefinition{"elysium:threat/environmental_disaster", ThreatKind::EnvironmentalDisaster, 2.5f, true}
};

constexpr std::array kVehicles{
    VehicleDefinition{"elysium:vehicle/scout_rover", "fast surface survey", 12.0f, 1.3f},
    VehicleDefinition{"elysium:vehicle/cargo_crawler", "bulk hauling", 96.0f, 0.55f},
    VehicleDefinition{"elysium:vehicle/mining_rig", "mobile drill/extractor", 48.0f, 0.4f},
    VehicleDefinition{"elysium:vehicle/amphibious_skiff", "ocean/coast travel", 32.0f, 0.9f},
    VehicleDefinition{"elysium:vehicle/hover_sled", "rough-terrain late game", 20.0f, 1.5f},
    VehicleDefinition{"elysium:vehicle/siege_hauler", "defense/logistics", 160.0f, 0.35f}
};

template <class T, std::size_t N>
const T* findById(const std::array<T, N>& values, std::string_view id) {
    const auto it = std::find_if(values.begin(), values.end(), [id](const T& value) { return value.id == id; });
    return it == values.end() ? nullptr : &*it;
}

} // namespace

std::span<const LaborDefinition> laborCatalog() { return kLabors; }
std::span<const MachineDefinition> machineCatalog() { return kMachines; }
std::span<const InstitutionDefinition> institutionCatalog() { return kInstitutions; }
std::span<const ThreatDefinition> threatCatalog() { return kThreats; }
std::span<const VehicleDefinition> vehicleCatalog() { return kVehicles; }

const LaborDefinition* findLabor(std::string_view id) { return findById(kLabors, id); }
const MachineDefinition* findMachine(std::string_view id) { return findById(kMachines, id); }

} // namespace elysium::fortress
