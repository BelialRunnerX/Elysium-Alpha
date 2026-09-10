// Intended function: imported travel implementation for Vehicles; preserves the agent-authored subsystem contract for later integration/debugging.
#include "travel/Vehicles.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace elysium {

namespace {
const std::vector<VehicleDefinition> kVehicles{
    {VehicleType::ScoutRover,"Scout Rover",18.0f,8,35.0f,0.70f,0.35f,false,false,0.25f,2.2f,8.0f,180.0f,0.0f},
    {VehicleType::CargoCrawler,"Cargo Crawler",7.0f,48,20.0f,0.45f,0.25f,false,false,0.35f,2.0f,15.0f,500.0f,0.0f},
    {VehicleType::MiningRig,"Mining Rig",4.5f,24,16.0f,0.35f,0.20f,false,false,0.45f,1.8f,32.0f,800.0f,0.40f},
    {VehicleType::AmphibiousSkiff,"Amphibious Skiff",13.0f,18,18.0f,0.60f,250.0f,true,false,0.20f,1.8f,14.0f,300.0f,0.0f},
    {VehicleType::HoverSled,"Hover Sled",22.0f,14,45.0f,1.00f,1.50f,false,true,0.15f,2.5f,28.0f,260.0f,0.0f},
    {VehicleType::SiegeHauler,"Siege Hauler",5.5f,64,22.0f,0.50f,0.30f,false,false,0.45f,2.1f,38.0f,1200.0f,0.25f}
};

void normalizeCargo(VehicleState& v) {
    std::sort(v.cargo.begin(),v.cargo.end(),[](const auto& a,const auto& b){return a.itemId<b.itemId;});
    std::vector<VehicleCargoStack> out;
    for(const auto& s:v.cargo) {
        if(s.itemId<=0 || s.count<=0) continue;
        if(!out.empty() && out.back().itemId==s.itemId) out.back().count+=s.count;
        else out.push_back(s);
    }
    v.cargo.swap(out);
}
} // namespace

const std::vector<VehicleDefinition>& vehicleRegistry() { return kVehicles; }

const VehicleDefinition& vehicleDefinition(VehicleType type) {
    const auto it=std::find_if(kVehicles.begin(),kVehicles.end(),[&](const auto& v){return v.type==type;});
    if(it==kVehicles.end()) throw std::invalid_argument("unknown VehicleType");
    return *it;
}

const char* vehicleMoveFailureName(VehicleMoveFailure failure) {
    switch(failure) {
        case VehicleMoveFailure::None:return "none";
        case VehicleMoveFailure::InvalidState:return "invalid_state";
        case VehicleMoveFailure::WrongPlanet:return "wrong_planet";
        case VehicleMoveFailure::SlopeTooSteep:return "slope_too_steep";
        case VehicleMoveFailure::TerrainTooRough:return "terrain_too_rough";
        case VehicleMoveFailure::WaterTooDeep:return "water_too_deep";
        case VehicleMoveFailure::GravityUnsupported:return "gravity_unsupported";
        case VehicleMoveFailure::WeatherUnsafe:return "weather_unsafe";
        case VehicleMoveFailure::CargoOverCapacity:return "cargo_over_capacity";
        case VehicleMoveFailure::InsufficientEnergy:return "insufficient_energy";
        case VehicleMoveFailure::NoCrewOrAutonomy:return "no_crew_or_autonomy";
    }
    return "unknown";
}

VehicleState VehicleSystem::create(std::uint64_t stableId,VehicleType type,std::uint32_t systemId,std::uint32_t planetId,SurfaceCellAddress address) const {
    const auto& def=vehicleDefinition(type);
    VehicleState out;
    out.stableId=stableId;
    out.type=type;
    out.systemId=systemId;
    out.planetId=planetId;
    out.address=address;
    out.routeTarget=address;
    out.hull=def.hull;
    out.energy=100.0f;
    return out;
}

VehicleMoveResult VehicleSystem::evaluateMove(const VehicleState& vehicle,const VehicleMoveContext& c) const {
    VehicleMoveResult out;
    auto fail=[&](VehicleMoveFailure f,std::string r){out.failure=f;out.reason=std::move(r);return out;};
    const auto& def=vehicleDefinition(vehicle.type);
    if(vehicle.stableId==0 || !std::isfinite(vehicle.hull) || vehicle.hull<=0.0f || !std::isfinite(vehicle.energy) || vehicle.energy<0.0f)
        return fail(VehicleMoveFailure::InvalidState,"Vehicle state is not operational.");
    if(vehicle.systemId!=c.systemId || vehicle.planetId!=c.planetId) return fail(VehicleMoveFailure::WrongPlanet,"Route step belongs to another world.");
    if(vehicle.crewStableId==0 && !vehicle.autonomous) return fail(VehicleMoveFailure::NoCrewOrAutonomy,"Vehicle requires a driver or autonomy module.");
    if(c.gravityG<def.minGravityG || c.gravityG>def.maxGravityG) return fail(VehicleMoveFailure::GravityUnsupported,"Gravity lies outside the propulsion/suspension envelope.");
    if(!def.hover && c.slopeDegrees>def.maxSlopeDegrees) return fail(VehicleMoveFailure::SlopeTooSteep,"Requested route segment exceeds maximum slope.");
    if(!def.hover && c.roughness>def.maxRoughness) return fail(VehicleMoveFailure::TerrainTooRough,"Terrain roughness exceeds this vehicle's ground-contact limit.");
    if(!def.amphibious && c.waterDepthMeters>def.maxWaterDepthMeters) return fail(VehicleMoveFailure::WaterTooDeep,"Water depth exceeds the vehicle's ford limit.");
    if(c.weatherSeverity>0.90f && vehicle.type==VehicleType::AmphibiousSkiff) return fail(VehicleMoveFailure::WeatherUnsafe,"Sea/weather state exceeds skiff operating limits.");
    if(cargoUsedSlots(vehicle)>def.cargoSlots) return fail(VehicleMoveFailure::CargoOverCapacity,"Cargo exceeds the vehicle hold.");
    const float terrainPenalty=def.hover?0.90f:std::clamp(1.0f-c.roughness*0.45f-c.slopeDegrees/120.0f,0.25f,1.0f);
    const float weatherPenalty=std::clamp(1.0f-c.weatherSeverity*0.35f,0.50f,1.0f);
    out.speedMps=def.cruiseSpeedMps*terrainPenalty*weatherPenalty;
    out.energyCost=def.energyPerKm*std::max(0.0f,c.segmentMeters)/1000.0f*(1.0f+std::clamp(c.weatherSeverity,0.0f,1.0f)*0.25f);
    if(vehicle.energy+1e-6f<out.energyCost) return fail(VehicleMoveFailure::InsufficientEnergy,"Vehicle does not have enough energy for the route segment.");
    out.suspicionGenerated=def.industrialSuspicionPerKm*std::max(0.0f,c.segmentMeters)/1000.0f;
    out.allowed=true;
    out.reason="Shared-navigation route segment accepted.";
    return out;
}

bool VehicleSystem::commitMove(VehicleState& vehicle,const VehicleMoveContext& context,VehicleMoveResult* result) const {
    auto r=evaluateMove(vehicle,context);
    if(result) *result=r;
    if(!r.allowed) return false;
    vehicle.address=context.to;
    vehicle.energy=std::max(0.0f,vehicle.energy-r.energyCost);
    return true;
}

int VehicleSystem::cargoUsedSlots(const VehicleState& vehicle) {
    int n=0;
    for(const auto& s:vehicle.cargo) if(s.itemId>0 && s.count>0) ++n;
    return n;
}

int VehicleSystem::cargoInsert(VehicleState& vehicle,int itemId,int count) {
    if(itemId<=0 || count<=0) return 0;
    normalizeCargo(vehicle);
    auto it=std::lower_bound(vehicle.cargo.begin(),vehicle.cargo.end(),itemId,[](const auto& s,int id){return s.itemId<id;});
    if(it!=vehicle.cargo.end() && it->itemId==itemId){it->count+=count;return count;}
    if(cargoUsedSlots(vehicle)>=vehicleDefinition(vehicle.type).cargoSlots) return 0;
    vehicle.cargo.insert(it,{itemId,count});
    return count;
}

int VehicleSystem::cargoExtract(VehicleState& vehicle,int itemId,int count) {
    if(itemId<=0 || count<=0) return 0;
    normalizeCargo(vehicle);
    auto it=std::lower_bound(vehicle.cargo.begin(),vehicle.cargo.end(),itemId,[](const auto& s,int id){return s.itemId<id;});
    if(it==vehicle.cargo.end() || it->itemId!=itemId) return 0;
    const int taken=std::min(count,it->count);
    it->count-=taken;
    if(it->count<=0) vehicle.cargo.erase(it);
    return taken;
}

} // namespace elysium
