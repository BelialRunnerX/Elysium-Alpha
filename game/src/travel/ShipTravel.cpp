// Intended function: imported travel implementation for ShipTravel; preserves the agent-authored subsystem contract for later integration/debugging.
#include "travel/ShipTravel.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace elysium {

namespace {

const std::vector<ShipModuleDefinition> kModules{
    {.id=ShipModuleId::WarpDriveI,.name="Warp Drive I",.slot=ShipModuleSlot::Propulsion,.tier=2,.mass=2.0f,.powerDraw=2.0f,.jumpRangeLy=100.0f},
    {.id=ShipModuleId::WarpDriveII,.name="Warp Drive II",.slot=ShipModuleSlot::Propulsion,.tier=3,.mass=3.0f,.powerDraw=3.0f,.jumpRangeLy=200.0f},
    {.id=ShipModuleId::WarpDriveIII,.name="Warp Drive III",.slot=ShipModuleSlot::Propulsion,.tier=4,.mass=4.0f,.powerDraw=4.0f,.jumpRangeLy=400.0f},
    {.id=ShipModuleId::WarpDriveIV,.name="Warp Drive IV",.slot=ShipModuleSlot::Propulsion,.tier=5,.mass=5.0f,.powerDraw=5.0f,.jumpRangeLy=800.0f},
    {.id=ShipModuleId::PulseEngine,.name="Pulse Engine",.slot=ShipModuleSlot::Propulsion,.tier=1,.mass=1.5f,.powerDraw=1.0f,.pulseTransit=true},
    {.id=ShipModuleId::AtmosphericThrusters,.name="Atmospheric Thrusters",.slot=ShipModuleSlot::Propulsion,.tier=1,.mass=2.0f,.powerDraw=1.5f,.handlingBonus=1.0f},
    {.id=ShipModuleId::VectorJets,.name="Vector Jets",.slot=ShipModuleSlot::Propulsion,.tier=1,.mass=1.0f,.powerDraw=0.6f,.handlingBonus=0.45f},
    {.id=ShipModuleId::FuelTank,.name="Fuel Tank",.slot=ShipModuleSlot::Utility,.tier=1,.mass=1.5f,.fuelCapacityBonus=4},
    {.id=ShipModuleId::CargoRack,.name="Cargo Rack",.slot=ShipModuleSlot::Utility,.tier=1,.mass=2.0f,.cargoSlotBonus=12},
    {.id=ShipModuleId::SurveyScanner,.name="Survey Scanner",.slot=ShipModuleSlot::Science,.tier=1,.mass=0.5f,.powerDraw=0.4f,.scannerGradeBonus=1},
    {.id=ShipModuleId::DeepScanner,.name="Deep Scanner",.slot=ShipModuleSlot::Science,.tier=2,.mass=0.8f,.powerDraw=0.8f,.scannerGradeBonus=1,.deepScan=true},
    {.id=ShipModuleId::AtmosphereSampler,.name="Atmosphere Sampler",.slot=ShipModuleSlot::Science,.tier=1,.mass=0.4f,.powerDraw=0.3f,.atmosphereScan=true},
    {.id=ShipModuleId::HullPlating,.name="Hull Plating",.slot=ShipModuleSlot::Defense,.tier=1,.mass=4.0f,.hullBonus=200.0f},
    {.id=ShipModuleId::ThermalShield,.name="Thermal Shield",.slot=ShipModuleSlot::Defense,.tier=2,.mass=1.5f,.powerDraw=1.0f,.thermalEntryMitigation=0.65f},
    {.id=ShipModuleId::RadiationBaffle,.name="Radiation Baffle",.slot=ShipModuleSlot::Defense,.tier=2,.mass=2.0f,.powerDraw=0.5f,.radiationEntryMitigation=0.65f},
    {.id=ShipModuleId::AutopilotComputer,.name="Autopilot Computer",.slot=ShipModuleSlot::Control,.tier=1,.mass=0.4f,.powerDraw=0.2f,.autopilot=true},
    {.id=ShipModuleId::NavigationCore,.name="Navigation Core",.slot=ShipModuleSlot::Control,.tier=2,.mass=0.7f,.powerDraw=0.4f},
    {.id=ShipModuleId::EmergencyBeacon,.name="Emergency Beacon",.slot=ShipModuleSlot::Safety,.tier=1,.mass=0.3f,.powerDraw=0.1f,.emergencyRecovery=true},
    {.id=ShipModuleId::DockingCollar,.name="Docking Collar",.slot=ShipModuleSlot::Utility,.tier=1,.mass=1.0f,.powerDraw=0.1f,.docking=true},
    {.id=ShipModuleId::DroneBay,.name="Drone Bay",.slot=ShipModuleSlot::Utility,.tier=3,.mass=2.5f,.powerDraw=1.2f,.droneSupport=true},
    {.id=ShipModuleId::HabitationPod,.name="Habitation Pod",.slot=ShipModuleSlot::Utility,.tier=3,.mass=3.0f,.powerDraw=0.8f,.cargoSlotBonus=4,.habitation=true},
    {.id=ShipModuleId::SmallRefinery,.name="Small Refinery",.slot=ShipModuleSlot::Industry,.tier=3,.mass=3.5f,.powerDraw=2.0f,.fieldRefinery=true},
    {.id=ShipModuleId::SmugglerHold,.name="Smuggler Hold",.slot=ShipModuleSlot::Faction,.tier=3,.mass=2.5f,.cargoSlotBonus=6,.inspectionExposureMultiplier=0.65f}
};

bool finite01(float v) { return std::isfinite(v) && v>=0.0f && v<=1.0f; }

void normalizeCargo(ShipState& ship) {
    std::sort(ship.cargo.begin(),ship.cargo.end(),[](const auto& a,const auto& b){return a.itemId<b.itemId;});
    std::vector<ShipCargoStack> normalized;
    for(const auto& stack:ship.cargo) {
        if(stack.itemId<=0 || stack.count<=0) continue;
        if(!normalized.empty() && normalized.back().itemId==stack.itemId) normalized.back().count+=stack.count;
        else normalized.push_back(stack);
    }
    ship.cargo.swap(normalized);
}

bool hasModule(const ShipState& ship, ShipModuleId id) {
    return std::find(ship.installedModules.begin(),ship.installedModules.end(),id)!=ship.installedModules.end();
}

float cargoFill(const ShipState& ship,const ShipDerivedStats& stats) {
    if(stats.cargoSlots<=0) return 1.0f;
    return std::clamp(static_cast<float>(shipCargoUsedSlots(ship))/static_cast<float>(stats.cargoSlots),0.0f,1.0f);
}

} // namespace

const std::vector<ShipModuleDefinition>& shipModuleRegistry() { return kModules; }

const ShipModuleDefinition* shipModuleDefinition(ShipModuleId id) {
    const auto it=std::find_if(kModules.begin(),kModules.end(),[&](const auto& m){return m.id==id;});
    return it==kModules.end()?nullptr:&*it;
}

ShipDerivedStats deriveShipStats(const ShipState& ship) {
    ShipDerivedStats out;
    for(const auto id:ship.installedModules) {
        const auto* def=shipModuleDefinition(id);
        if(!def) continue;
        out.jumpRangeLy=std::max(out.jumpRangeLy,def->jumpRangeLy);
        out.fuelCapacity+=def->fuelCapacityBonus;
        out.cargoSlots+=def->cargoSlotBonus;
        out.maxHull+=def->hullBonus;
        out.atmosphericHandling+=def->handlingBonus;
        out.scannerGrade+=def->scannerGradeBonus;
        out.moduleMass+=def->mass;
        out.thermalEntryMitigation=1.0f-(1.0f-out.thermalEntryMitigation)*(1.0f-std::clamp(def->thermalEntryMitigation,0.0f,0.999f));
        out.radiationEntryMitigation=1.0f-(1.0f-out.radiationEntryMitigation)*(1.0f-std::clamp(def->radiationEntryMitigation,0.0f,0.999f));
        out.pulseTransit=out.pulseTransit||def->pulseTransit;
        out.docking=out.docking||def->docking;
        out.autopilot=out.autopilot||def->autopilot;
        out.emergencyRecovery=out.emergencyRecovery||def->emergencyRecovery;
        out.deepScan=out.deepScan||def->deepScan;
        out.atmosphereScan=out.atmosphereScan||def->atmosphereScan;
        out.droneSupport=out.droneSupport||def->droneSupport;
        out.habitation=out.habitation||def->habitation;
        out.fieldRefinery=out.fieldRefinery||def->fieldRefinery;
        out.inspectionExposureMultiplier*=std::clamp(def->inspectionExposureMultiplier,0.1f,2.0f);
    }
    out.fuelCapacity=std::clamp(out.fuelCapacity,4,24);
    out.cargoSlots=std::clamp(out.cargoSlots,12,96);
    out.maxHull=std::clamp(out.maxHull,200.0f,1200.0f);
    out.atmosphericHandling=std::clamp(out.atmosphericHandling,0.0f,3.0f);
    out.scannerGrade=std::clamp(out.scannerGrade,1,4);
    return out;
}

bool validateShipState(const ShipState& ship,std::string* error) {
    auto fail=[&](const char* e){if(error)*error=e;return false;};
    if(ship.stableId==0) return fail("ship stableId is zero");
    const auto stats=deriveShipStats(ship);
    if(ship.fuelCells<0 || ship.fuelCells>stats.fuelCapacity) return fail("ship fuel exceeds derived capacity");
    if(!std::isfinite(ship.hull) || ship.hull<0.0f || ship.hull>stats.maxHull+0.001f) return fail("ship hull outside derived range");
    if(!finite01(ship.maintenance)) return fail("ship maintenance outside [0,1]");
    for(const auto id:ship.installedModules) if(!shipModuleDefinition(id)) return fail("unknown ship module id");
    for(std::size_t i=0;i<ship.installedModules.size();++i)
        for(std::size_t j=i+1;j<ship.installedModules.size();++j)
            if(ship.installedModules[i]==ship.installedModules[j]) return fail("duplicate ship module id");
    if(shipCargoUsedSlots(ship)>stats.cargoSlots) return fail("ship cargo exceeds slot capacity");
    for(const auto& s:ship.cargo) if(s.itemId<=0 || s.count<=0) return fail("invalid ship cargo stack");
    if(ship.transit.active) {
        if(ship.transit.durationSeconds<=0.0f || !std::isfinite(ship.transit.durationSeconds) || !std::isfinite(ship.transit.elapsedSeconds)) return fail("invalid active transit timing");
        if(ship.location.kind!=ship.transit.phase) return fail("ship location/transit phase mismatch");
    }
    return true;
}

int shipCargoCount(const ShipState& ship,int itemId) {
    int total=0;
    for(const auto& s:ship.cargo) if(s.itemId==itemId) total+=s.count;
    return total;
}

int shipCargoUsedSlots(const ShipState& ship) {
    int slots=0;
    for(const auto& s:ship.cargo) if(s.itemId>0 && s.count>0) ++slots;
    return slots;
}

int shipCargoInsert(ShipState& ship,int itemId,int count) {
    if(itemId<=0 || count<=0) return 0;
    normalizeCargo(ship);
    auto it=std::lower_bound(ship.cargo.begin(),ship.cargo.end(),itemId,[](const auto& s,int id){return s.itemId<id;});
    if(it!=ship.cargo.end() && it->itemId==itemId) {it->count+=count;return count;}
    if(shipCargoUsedSlots(ship)>=deriveShipStats(ship).cargoSlots) return 0;
    ship.cargo.insert(it,{itemId,count});
    return count;
}

int shipCargoExtract(ShipState& ship,int itemId,int count) {
    if(itemId<=0 || count<=0) return 0;
    normalizeCargo(ship);
    auto it=std::lower_bound(ship.cargo.begin(),ship.cargo.end(),itemId,[](const auto& s,int id){return s.itemId<id;});
    if(it==ship.cargo.end() || it->itemId!=itemId) return 0;
    const int taken=std::min(count,it->count);
    it->count-=taken;
    if(it->count<=0) ship.cargo.erase(it);
    return taken;
}

const char* travelFailureName(TravelFailure failure) {
    switch(failure) {
        case TravelFailure::None:return "none";
        case TravelFailure::Busy:return "busy";
        case TravelFailure::NotOnSurface:return "not_on_surface";
        case TravelFailure::NotInOrbit:return "not_in_orbit";
        case TravelFailure::MissingAtmosphericThrusters:return "missing_atmospheric_thrusters";
        case TravelFailure::MissingPulseEngine:return "missing_pulse_engine";
        case TravelFailure::MissingWarpDrive:return "missing_warp_drive";
        case TravelFailure::MissingDockingCollar:return "missing_docking_collar";
        case TravelFailure::CargoOverCapacity:return "cargo_over_capacity";
        case TravelFailure::FuelCapacityInvalid:return "fuel_capacity_invalid";
        case TravelFailure::InsufficientFuel:return "insufficient_fuel";
        case TravelFailure::RangeExceeded:return "range_exceeded";
        case TravelFailure::GravityTooHigh:return "gravity_too_high";
        case TravelFailure::StormTooSevere:return "storm_too_severe";
        case TravelFailure::HullTooDamaged:return "hull_too_damaged";
        case TravelFailure::ThermalEntryUnsafe:return "thermal_entry_unsafe";
        case TravelFailure::RadiationEntryUnsafe:return "radiation_entry_unsafe";
        case TravelFailure::DestinationInvalid:return "destination_invalid";
        case TravelFailure::LoaderUnavailable:return "loader_unavailable";
        case TravelFailure::LoaderUnpowered:return "loader_unpowered";
        case TravelFailure::LoaderNotDocked:return "loader_not_docked";
        case TravelFailure::CargoFull:return "cargo_full";
        case TravelFailure::InvalidState:return "invalid_state";
    }
    return "unknown";
}

TravelCheck ShipTravelSystem::checkTakeoff(const ShipState& ship,const TakeoffEnvironment& environment) const {
    TravelCheck out;
    auto fail=[&](TravelFailure f,std::string r){out.failure=f;out.reason=std::move(r);return out;};
    if(ship.transit.active) return fail(TravelFailure::Busy,"Ship is already in a travel transition.");
    if(ship.location.kind!=ShipLocationKind::Surface) return fail(TravelFailure::NotOnSurface,"Takeoff requires a surface location.");
    std::string stateError;
    if(!validateShipState(ship,&stateError)) return fail(TravelFailure::InvalidState,stateError);
    const auto stats=deriveShipStats(ship);
    if(!hasModule(ship,ShipModuleId::AtmosphericThrusters)) return fail(TravelFailure::MissingAtmosphericThrusters,"Atmospheric Thrusters are required for takeoff.");
    if(shipCargoUsedSlots(ship)>stats.cargoSlots) return fail(TravelFailure::CargoOverCapacity,"Cargo exceeds the derived hold capacity.");
    if(ship.hull<stats.maxHull*MinimumOperationalHullFraction) return fail(TravelFailure::HullTooDamaged,"Hull integrity is below the takeoff safety floor.");
    if(!std::isfinite(environment.gravityG) || environment.gravityG<=0.0f) return fail(TravelFailure::DestinationInvalid,"Planet gravity is invalid.");
    const float fill=std::max(cargoFill(ship,stats),std::clamp(environment.cargoMassFraction,0.0f,1.0f));
    const float required=environment.gravityG + fill*0.75f + std::clamp(environment.stormSeverity,0.0f,1.0f)*0.55f;
    const float envelope=0.85f + stats.atmosphericHandling*0.85f;
    if(required>envelope+0.0001f) {
        if(environment.stormSeverity>0.65f && environment.gravityG+fill*0.75f<=envelope) return fail(TravelFailure::StormTooSevere,"Storm load pushes the craft outside its atmospheric handling envelope.");
        return fail(TravelFailure::GravityTooHigh,"Gravity/cargo load exceeds the ship's atmospheric handling envelope.");
    }
    out.allowed=true;
    out.reason="Takeoff envelope satisfied.";
    return out;
}

bool ShipTravelSystem::reject(TravelFailure failure,std::string_view detail,std::string* reason) {
    ++telemetry_.rejectedCommands;
    if(reason) *reason=std::string(travelFailureName(failure))+": "+std::string(detail);
    return false;
}

bool ShipTravelSystem::beginTakeoff(ShipState& ship,const TakeoffEnvironment& environment,std::string* reason) {
    const auto check=checkTakeoff(ship,environment);
    if(!check.allowed) return reject(check.failure,check.reason,reason);
    ship.surfaceCargoLoaderStableId=0;
    ship.location.dockedObjectStableId=0;
    ship.location.kind=ShipLocationKind::Ascending;
    ship.transit={true,ShipLocationKind::Ascending,0.0f,TakeoffSeconds,ship.location.systemId,ship.location.planetId,ship.location.surfaceAddress,0,0.0f,0};
    ++telemetry_.takeoffsStarted;
    if(reason) *reason="takeoff_started";
    return true;
}

bool ShipTravelSystem::beginLanding(ShipState& ship,std::uint32_t targetPlanetId,SurfaceCellAddress targetSurfaceAddress,
                                    const TakeoffEnvironment& entry,std::string* reason) {
    if(ship.transit.active) return reject(TravelFailure::Busy,"Ship is already in transit.",reason);
    if(ship.location.kind!=ShipLocationKind::Orbit || ship.location.planetId!=targetPlanetId)
        return reject(TravelFailure::NotInOrbit,"Landing requires orbit around the target planet.",reason);
    const auto stats=deriveShipStats(ship);
    if(!hasModule(ship,ShipModuleId::AtmosphericThrusters)) return reject(TravelFailure::MissingAtmosphericThrusters,"Atmospheric Thrusters are required for entry.",reason);
    if(ship.hull<stats.maxHull*MinimumOperationalHullFraction) return reject(TravelFailure::HullTooDamaged,"Hull integrity is below the atmospheric-entry floor.",reason);
    if(std::clamp(entry.thermalSeverity,0.0f,1.0f)*(1.0f-stats.thermalEntryMitigation)>0.70f)
        return reject(TravelFailure::ThermalEntryUnsafe,"Thermal entry load exceeds installed mitigation.",reason);
    if(std::clamp(entry.radiationSeverity,0.0f,1.0f)*(1.0f-stats.radiationEntryMitigation)>0.70f)
        return reject(TravelFailure::RadiationEntryUnsafe,"Radiological entry load exceeds installed mitigation.",reason);
    ship.location.kind=ShipLocationKind::Descending;
    ship.transit={true,ShipLocationKind::Descending,0.0f,LandingSeconds,ship.location.systemId,targetPlanetId,targetSurfaceAddress,0,0.0f,0};
    ++telemetry_.landingsStarted;
    if(reason) *reason="landing_started";
    return true;
}

bool ShipTravelSystem::beginInSystemTravel(ShipState& ship,std::uint32_t targetPlanetId,float transitSeconds,std::string* reason) {
    if(ship.transit.active) return reject(TravelFailure::Busy,"Ship is already in transit.",reason);
    if(ship.location.kind!=ShipLocationKind::Orbit) return reject(TravelFailure::NotInOrbit,"In-system flight starts from orbit.",reason);
    const auto stats=deriveShipStats(ship);
    if(!stats.pulseTransit) return reject(TravelFailure::MissingPulseEngine,"Pulse Engine is required for in-system transit.",reason);
    if(targetPlanetId==ship.location.planetId) return reject(TravelFailure::DestinationInvalid,"Target planet is the current planet.",reason);
    if(ship.fuelCells<1) return reject(TravelFailure::InsufficientFuel,"In-system transit requires one fuel cell in the prototype economy.",reason);
    ship.fuelCells-=1;
    ship.location.kind=ShipLocationKind::InSystemTransit;
    ship.transit={true,ShipLocationKind::InSystemTransit,0.0f,std::max(MinInSystemSeconds,transitSeconds),ship.location.systemId,targetPlanetId,{},0,0.0f,1};
    ++telemetry_.inSystemTrips;
    ++telemetry_.fuelCellsConsumed;
    if(reason) *reason="in_system_transit_started";
    return true;
}

int ShipTravelSystem::warpFuelCost(float distanceLy) {
    if(!std::isfinite(distanceLy) || distanceLy<=0.0f) return 0;
    return std::max(1,static_cast<int>(std::ceil(distanceLy/100.0f)));
}

bool ShipTravelSystem::beginWarpTravel(ShipState& ship,std::uint32_t targetSystemId,float distanceLy,std::string* reason) {
    if(ship.transit.active) return reject(TravelFailure::Busy,"Ship is already in transit.",reason);
    if(ship.location.kind!=ShipLocationKind::Orbit) return reject(TravelFailure::NotInOrbit,"Warp starts from orbit.",reason);
    const auto stats=deriveShipStats(ship);
    if(stats.jumpRangeLy<=0.0f) return reject(TravelFailure::MissingWarpDrive,"A Warp Drive is required.",reason);
    if(targetSystemId==ship.location.systemId || !std::isfinite(distanceLy) || distanceLy<=0.0f) return reject(TravelFailure::DestinationInvalid,"Warp destination is invalid.",reason);
    if(distanceLy>stats.jumpRangeLy+0.001f) return reject(TravelFailure::RangeExceeded,"Requested jump exceeds the installed Warp Drive range.",reason);
    const int cost=warpFuelCost(distanceLy);
    if(ship.fuelCells<cost) return reject(TravelFailure::InsufficientFuel,"Not enough warp cells for this distance.",reason);
    ship.fuelCells-=cost;
    ship.location.kind=ShipLocationKind::WarpTransit;
    ship.transit={true,ShipLocationKind::WarpTransit,0.0f,WarpSeconds,targetSystemId,0,{},0,distanceLy,cost};
    ++telemetry_.warpTrips;
    telemetry_.fuelCellsConsumed+=cost;
    if(reason) *reason="warp_transit_started";
    return true;
}

bool ShipTravelSystem::dock(ShipState& ship,std::uint64_t orbitalStableId,std::string* reason) {
    if(ship.transit.active) return reject(TravelFailure::Busy,"Ship is already in transit.",reason);
    if(ship.location.kind!=ShipLocationKind::Orbit) return reject(TravelFailure::NotInOrbit,"Docking requires orbit.",reason);
    if(!deriveShipStats(ship).docking) return reject(TravelFailure::MissingDockingCollar,"Docking Collar is required.",reason);
    if(orbitalStableId==0) return reject(TravelFailure::DestinationInvalid,"Dock target StableId is zero.",reason);
    ship.location.kind=ShipLocationKind::Docked;
    ship.location.dockedObjectStableId=orbitalStableId;
    if(reason) *reason="docked";
    return true;
}

bool ShipTravelSystem::undock(ShipState& ship,std::string* reason) {
    if(ship.transit.active) return reject(TravelFailure::Busy,"Ship is already in transit.",reason);
    if(ship.location.kind!=ShipLocationKind::Docked) return reject(TravelFailure::NotInOrbit,"Ship is not docked.",reason);
    ship.location.kind=ShipLocationKind::Orbit;
    ship.location.dockedObjectStableId=0;
    if(reason) *reason="undocked";
    return true;
}

void ShipTravelSystem::update(ShipState& ship,float dt) {
    if(!ship.transit.active || !std::isfinite(dt) || dt<=0.0f) return;
    ship.transit.elapsedSeconds=std::min(ship.transit.durationSeconds,ship.transit.elapsedSeconds+dt);
    if(ship.transit.elapsedSeconds+1e-6f<ship.transit.durationSeconds) return;

    const auto phase=ship.transit.phase;
    if(phase==ShipLocationKind::Ascending) {
        ship.location.kind=ShipLocationKind::Orbit;
        ++telemetry_.takeoffsCompleted;
    } else if(phase==ShipLocationKind::Descending) {
        ship.location.kind=ShipLocationKind::Surface;
        ship.location.planetId=ship.transit.targetPlanetId;
        ship.location.surfaceAddress=ship.transit.targetSurfaceAddress;
        ++telemetry_.landingsCompleted;
    } else if(phase==ShipLocationKind::InSystemTransit) {
        ship.location.kind=ShipLocationKind::Orbit;
        ship.location.planetId=ship.transit.targetPlanetId;
        ship.location.dockedObjectStableId=0;
    } else if(phase==ShipLocationKind::WarpTransit) {
        ship.location.kind=ShipLocationKind::Orbit;
        ship.location.systemId=ship.transit.targetSystemId;
        ship.location.planetId=0;
        ship.location.dockedObjectStableId=0;
    }
    ship.transit={};
}

bool ShipTravelSystem::bindSurfaceCargoLoader(ShipState& ship,const SurfaceInfrastructure& infrastructure,std::uint64_t loaderStableId,std::string* reason) const {
    if(ship.location.kind!=ShipLocationKind::Surface) {if(reason)*reason="ship_not_on_surface";return false;}
    const auto* loader=infrastructure.find(loaderStableId);
    if(!loader || loader->type!=MachineType::CargoLoader) {if(reason)*reason="loader_unavailable";return false;}
    if(!loader->enabled || !loader->powered) {if(reason)*reason="loader_unpowered";return false;}
    ship.surfaceCargoLoaderStableId=loaderStableId;
    if(reason)*reason="loader_bound";
    return true;
}

int ShipTravelSystem::loadFromSurfaceCargoLoader(ShipState& ship,SurfaceInfrastructure& infrastructure,SurfaceIndustrySystem& industry,
                                                 int itemId,int count,std::string* reason) {
    if(count<=0) return 0;
    auto* loader=infrastructure.find(ship.surfaceCargoLoaderStableId);
    if(!loader || loader->type!=MachineType::CargoLoader) {reject(TravelFailure::LoaderUnavailable,"Bound Cargo Loader is unavailable.",reason);return 0;}
    if(ship.location.kind!=ShipLocationKind::Surface) {reject(TravelFailure::LoaderNotDocked,"Ship must be on the surface and loader-bound.",reason);return 0;}
    if(!loader->enabled || !loader->powered) {reject(TravelFailure::LoaderUnpowered,"Cargo Loader has no power.",reason);return 0;}
    const auto beforeShip=shipCargoCount(ship,itemId);
    if(shipCargoInsert(ship,itemId,count)!=count) {if(reason)*reason="cargo_full";return 0;}
    const int taken=industry.extract(infrastructure,loader->stableId,itemId,count);
    if(taken!=count) {
        shipCargoExtract(ship,itemId,count);
        if(taken>0) industry.insert(infrastructure,loader->stableId,itemId,taken);
        if(reason)*reason="loader_source_shortfall";
        return 0;
    }
    telemetry_.cargoItemsLoaded+=count;
    if(reason)*reason="loaded";
    (void)beforeShip;
    return count;
}

int ShipTravelSystem::unloadToSurfaceCargoLoader(ShipState& ship,SurfaceInfrastructure& infrastructure,SurfaceIndustrySystem& industry,
                                                 int itemId,int count,std::string* reason) {
    if(count<=0) return 0;
    auto* loader=infrastructure.find(ship.surfaceCargoLoaderStableId);
    if(!loader || loader->type!=MachineType::CargoLoader) {reject(TravelFailure::LoaderUnavailable,"Bound Cargo Loader is unavailable.",reason);return 0;}
    if(ship.location.kind!=ShipLocationKind::Surface) {reject(TravelFailure::LoaderNotDocked,"Ship must be on the surface and loader-bound.",reason);return 0;}
    if(!loader->enabled || !loader->powered) {reject(TravelFailure::LoaderUnpowered,"Cargo Loader has no power.",reason);return 0;}
    if(shipCargoCount(ship,itemId)<count) {if(reason)*reason="ship_source_shortfall";return 0;}
    const int taken=shipCargoExtract(ship,itemId,count);
    if(taken!=count) return 0;
    if(!industry.insert(infrastructure,loader->stableId,itemId,count)) {
        shipCargoInsert(ship,itemId,count);
        if(reason)*reason="loader_full";
        return 0;
    }
    telemetry_.cargoItemsUnloaded+=count;
    if(reason)*reason="unloaded";
    return count;
}

} // namespace elysium
