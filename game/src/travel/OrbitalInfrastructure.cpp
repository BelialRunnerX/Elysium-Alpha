// Intended function: imported travel implementation for OrbitalInfrastructure; preserves the agent-authored subsystem contract for later integration/debugging.
#include "travel/OrbitalInfrastructure.hpp"

#include "core/Determinism.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace elysium {

namespace {
const std::vector<OrbitalInfrastructureDefinition> kOrbitals{
    {OrbitalInfrastructureType::CargoDepot,"Orbital Cargo Depot",192,900.0f,8.0f,0.0f,0.0f,0.0f,1.0f,0.10f,false},
    {OrbitalInfrastructureType::SurveyRelay,"Survey Relay",12,450.0f,5.0f,0.50f,0.0f,0.0f,1.0f,0.08f,false},
    {OrbitalInfrastructureType::Drydock,"Drydock",48,1400.0f,18.0f,0.0f,0.0f,0.0f,1.0f,0.18f,true},
    {OrbitalInfrastructureType::OrbitalRefinery,"Orbital Refinery",96,1200.0f,24.0f,0.0f,2.0f,0.0f,1.0f,0.30f,false},
    {OrbitalInfrastructureType::DefensePlatform,"Defense Platform",24,1600.0f,30.0f,0.0f,0.0f,1.0f,1.0f,0.35f,false},
    {OrbitalInfrastructureType::GateAnchor,"Gate Anchor",32,1800.0f,40.0f,0.0f,0.0f,0.0f,2.0f,0.50f,false}
};

void normalizeCargo(OrbitalInfrastructureState& o) {
    std::sort(o.cargo.begin(),o.cargo.end(),[](const auto& a,const auto& b){return a.itemId<b.itemId;});
    std::vector<OrbitalCargoStack> out;
    for(const auto& s:o.cargo) {
        if(s.itemId<=0 || s.count<=0) continue;
        if(!out.empty() && out.back().itemId==s.itemId) out.back().count+=s.count;
        else out.push_back(s);
    }
    o.cargo.swap(out);
}

int usedSlots(const OrbitalInfrastructureState& o) {
    int n=0; for(const auto& s:o.cargo) if(s.itemId>0 && s.count>0) ++n; return n;
}
} // namespace

const std::vector<OrbitalInfrastructureDefinition>& orbitalInfrastructureRegistry() { return kOrbitals; }

const OrbitalInfrastructureDefinition& orbitalInfrastructureDefinition(OrbitalInfrastructureType type) {
    const auto it=std::find_if(kOrbitals.begin(),kOrbitals.end(),[&](const auto& d){return d.type==type;});
    if(it==kOrbitals.end()) throw std::invalid_argument("unknown OrbitalInfrastructureType");
    return *it;
}

OrbitalInfrastructureSystem::OrbitalInfrastructureSystem(std::uint64_t systemSeed):systemSeed_(systemSeed){}

std::uint64_t OrbitalInfrastructureSystem::allocateStableId() {
    std::uint64_t id{};
    do id=mix64(systemSeed_^0x4F52424954414C35ULL^nextSerial_++); while(id==0 || find(id));
    return id;
}

std::uint64_t OrbitalInfrastructureSystem::place(OrbitalInfrastructureType type,std::uint32_t systemId,std::uint32_t planetId,std::uint64_t ownerStableId) {
    const auto& def=orbitalInfrastructureDefinition(type);
    OrbitalInfrastructureState o;
    o.stableId=allocateStableId();o.type=type;o.systemId=systemId;o.planetId=planetId;o.ownerStableId=ownerStableId;o.hull=def.baseHull;
    objects_.push_back(o);
    std::sort(objects_.begin(),objects_.end(),[](const auto& a,const auto& b){return a.stableId<b.stableId;});
    return o.stableId;
}

bool OrbitalInfrastructureSystem::restore(const OrbitalInfrastructureState& state) {
    if(state.stableId==0 || find(state.stableId) || !std::isfinite(state.hull) || state.hull<0.0f ||
       !std::isfinite(state.maintenance) || state.maintenance<0.0f || state.maintenance>1.0f) return false;
    const auto& def=orbitalInfrastructureDefinition(state.type);
    if(state.hull>def.baseHull+0.001f) return false;
    OrbitalInfrastructureState copy=state;
    normalizeCargo(copy);
    if(usedSlots(copy)>def.cargoSlots) return false;
    for(const auto module:copy.serviceInventory) if(!shipModuleDefinition(module)) return false;
    objects_.push_back(std::move(copy));
    std::sort(objects_.begin(),objects_.end(),[](const auto& a,const auto& b){return a.stableId<b.stableId;});
    return true;
}

bool OrbitalInfrastructureSystem::remove(std::uint64_t stableId) {
    const auto it=std::find_if(objects_.begin(),objects_.end(),[&](const auto& o){return o.stableId==stableId;});
    if(it==objects_.end()) return false;
    objects_.erase(it); return true;
}

OrbitalInfrastructureState* OrbitalInfrastructureSystem::find(std::uint64_t stableId) {
    const auto it=std::lower_bound(objects_.begin(),objects_.end(),stableId,[](const auto& o,std::uint64_t id){return o.stableId<id;});
    return it!=objects_.end() && it->stableId==stableId?&*it:nullptr;
}
const OrbitalInfrastructureState* OrbitalInfrastructureSystem::find(std::uint64_t stableId) const {
    const auto it=std::lower_bound(objects_.begin(),objects_.end(),stableId,[](const auto& o,std::uint64_t id){return o.stableId<id;});
    return it!=objects_.end() && it->stableId==stableId?&*it:nullptr;
}

OrbitalServiceSummary OrbitalInfrastructureSystem::summary(std::uint32_t systemId,std::uint32_t planetId) const {
    OrbitalServiceSummary out;
    for(const auto& o:objects_) {
        if(!o.enabled || !o.powered || o.systemId!=systemId) continue;
        if(o.planetId!=0 && planetId!=0 && o.planetId!=planetId) continue;
        const auto& d=orbitalInfrastructureDefinition(o.type);
        ++out.poweredAssets;
        out.cargoSlots+=d.cargoSlots;
        out.surveyRangeBonus+=d.surveyRangeBonus;
        out.refineryThroughput+=d.refineryThroughput;
        out.defenseStrength+=d.defenseStrength;
        out.gateRangeMultiplier=std::max(out.gateRangeMultiplier,d.gateRangeMultiplier);
        out.exposure+=d.exposure;
        out.hasDrydock=out.hasDrydock||d.shipService;
    }
    return out;
}

int OrbitalInfrastructureSystem::cargoInsert(std::uint64_t stableId,int itemId,int count) {
    auto* o=find(stableId); if(!o || itemId<=0 || count<=0) return 0;
    normalizeCargo(*o);
    auto it=std::lower_bound(o->cargo.begin(),o->cargo.end(),itemId,[](const auto& s,int id){return s.itemId<id;});
    if(it!=o->cargo.end() && it->itemId==itemId){it->count+=count;return count;}
    if(usedSlots(*o)>=orbitalInfrastructureDefinition(o->type).cargoSlots) return 0;
    o->cargo.insert(it,{itemId,count}); return count;
}

int OrbitalInfrastructureSystem::cargoExtract(std::uint64_t stableId,int itemId,int count) {
    auto* o=find(stableId); if(!o || itemId<=0 || count<=0) return 0;
    normalizeCargo(*o);
    auto it=std::lower_bound(o->cargo.begin(),o->cargo.end(),itemId,[](const auto& s,int id){return s.itemId<id;});
    if(it==o->cargo.end() || it->itemId!=itemId) return 0;
    const int taken=std::min(count,it->count);it->count-=taken;if(it->count<=0)o->cargo.erase(it);return taken;
}

bool OrbitalInfrastructureSystem::installModuleAtDrydock(ShipState& ship,std::uint64_t drydockStableId,ShipModuleId module,std::string* error) {
    auto fail=[&](const char* e){if(error)*error=e;return false;};
    auto* dock=find(drydockStableId);
    if(!dock || dock->type!=OrbitalInfrastructureType::Drydock || !dock->enabled || !dock->powered) return fail("drydock unavailable or unpowered");
    if(ship.location.kind!=ShipLocationKind::Docked || ship.location.dockedObjectStableId!=drydockStableId) return fail("ship is not docked at this drydock");
    if(!shipModuleDefinition(module)) return fail("unknown module");
    if(std::find(ship.installedModules.begin(),ship.installedModules.end(),module)!=ship.installedModules.end()) return fail("module already installed");
    auto it=std::find(dock->serviceInventory.begin(),dock->serviceInventory.end(),module);
    if(it==dock->serviceInventory.end()) return fail("module not present in drydock service inventory");
    dock->serviceInventory.erase(it);
    ship.installedModules.push_back(module);
    std::sort(ship.installedModules.begin(),ship.installedModules.end(),[](auto a,auto b){return static_cast<int>(a)<static_cast<int>(b);});
    const auto stats=deriveShipStats(ship);
    ship.fuelCells=std::min(ship.fuelCells,stats.fuelCapacity);
    ship.hull=std::min(ship.hull,stats.maxHull);
    return true;
}

float OrbitalInfrastructureSystem::repairShipAtDrydock(ShipState& ship,std::uint64_t drydockStableId,float requestedHull,std::string* error) {
    auto* dock=find(drydockStableId);
    if(!dock || dock->type!=OrbitalInfrastructureType::Drydock || !dock->enabled || !dock->powered) {if(error)*error="drydock unavailable or unpowered";return 0.0f;}
    if(ship.location.kind!=ShipLocationKind::Docked || ship.location.dockedObjectStableId!=drydockStableId) {if(error)*error="ship is not docked at this drydock";return 0.0f;}
    const float need=std::max(0.0f,deriveShipStats(ship).maxHull-ship.hull);
    const float repaired=std::min(std::max(0.0f,requestedHull),need);
    ship.hull+=repaired;
    return repaired;
}

} // namespace elysium
