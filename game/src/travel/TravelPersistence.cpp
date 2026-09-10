// Intended function: imported travel implementation for TravelPersistence; preserves the agent-authored subsystem contract for later integration/debugging.
#include "travel/TravelPersistence.hpp"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <unordered_set>

namespace elysium {

namespace {

bool validFace(int f) { return f>=0 && f<6; }

bool validLocationKind(int k) { return k>=0 && k<=static_cast<int>(ShipLocationKind::Docked); }
bool validVehicleType(int k) { return k>=0 && k<=static_cast<int>(VehicleType::SiegeHauler); }
bool validOrbitalType(int k) { return k>=0 && k<=static_cast<int>(OrbitalInfrastructureType::GateAnchor); }

void writeAddress(std::ostream& out,const SurfaceCellAddress& a) {
    out<<static_cast<int>(a.face)<<' '<<a.u<<' '<<a.v<<' '<<a.radial;
}

bool readAddress(std::istream& in,SurfaceCellAddress& a) {
    int f{};
    if(!(in>>f>>a.u>>a.v>>a.radial) || !validFace(f)) return false;
    a.face=static_cast<CubeFace>(f);
    return true;
}

} // namespace

bool validateTravelPersistentState(const TravelPersistentState& state,std::string* error) {
    auto fail=[&](std::string e){if(error)*error=std::move(e);return false;};
    std::string shipError;
    if(!validateShipState(state.ship,&shipError)) return fail("ship: "+shipError);
    std::unordered_set<std::uint64_t> ids;
    if(!ids.insert(state.ship.stableId).second) return fail("duplicate ship stableId");
    for(const auto& v:state.vehicles) {
        if(v.stableId==0 || !ids.insert(v.stableId).second) return fail("duplicate/zero vehicle stableId");
        const auto& def=vehicleDefinition(v.type);
        if(!std::isfinite(v.hull) || v.hull<0 || v.hull>def.hull+0.001f) return fail("vehicle hull invalid");
        if(!std::isfinite(v.energy) || v.energy<0) return fail("vehicle energy invalid");
        if(!std::isfinite(v.maintenance) || v.maintenance<0 || v.maintenance>1) return fail("vehicle maintenance invalid");
        if(VehicleSystem::cargoUsedSlots(v)>def.cargoSlots) return fail("vehicle cargo exceeds capacity");
        for(const auto& s:v.cargo) if(s.itemId<=0 || s.count<=0) return fail("invalid vehicle cargo");
    }
    for(const auto& o:state.orbitalInfrastructure) {
        if(o.stableId==0 || !ids.insert(o.stableId).second) return fail("duplicate/zero orbital StableId");
        const auto& def=orbitalInfrastructureDefinition(o.type);
        if(!std::isfinite(o.hull) || o.hull<0 || o.hull>def.baseHull+0.001f) return fail("orbital hull invalid");
        if(!std::isfinite(o.maintenance) || o.maintenance<0 || o.maintenance>1) return fail("orbital maintenance invalid");
        int slots=0; for(const auto& s:o.cargo){if(s.itemId<=0||s.count<=0)return fail("invalid orbital cargo");++slots;}
        if(slots>def.cargoSlots) return fail("orbital cargo exceeds capacity");
        for(const auto m:o.serviceInventory) if(!shipModuleDefinition(m)) return fail("unknown drydock module content id");
    }
    return true;
}

std::string serializeTravelPersistentState(const TravelPersistentState& state) {
    std::string error;
    if(!validateTravelPersistentState(state,&error)) return {};
    std::ostringstream out;
    out<<std::setprecision(std::numeric_limits<float>::max_digits10);
    out<<"ELYSIUM_TRAVEL "<<TravelPersistentState::SchemaVersion<<'\n';
    const auto& s=state.ship;
    out<<"ship "<<s.stableId<<' '<<static_cast<int>(s.location.kind)<<' '<<s.location.systemId<<' '<<s.location.planetId<<' ';
    writeAddress(out,s.location.surfaceAddress);
    out<<' '<<s.location.dockedObjectStableId<<' '<<s.fuelCells<<' '<<s.hull<<' '<<s.maintenance<<' '<<s.surfaceCargoLoaderStableId<<'\n';
    out<<"modules "<<s.installedModules.size(); for(const auto m:s.installedModules) out<<' '<<static_cast<int>(m); out<<'\n';
    out<<"ship_cargo "<<s.cargo.size()<<'\n'; for(const auto& c:s.cargo) out<<c.itemId<<' '<<c.count<<'\n';
    out<<"transit "<<s.transit.active<<' '<<static_cast<int>(s.transit.phase)<<' '<<s.transit.elapsedSeconds<<' '<<s.transit.durationSeconds<<' '
       <<s.transit.targetSystemId<<' '<<s.transit.targetPlanetId<<' ';writeAddress(out,s.transit.targetSurfaceAddress);
    out<<' '<<s.transit.targetDockStableId<<' '<<s.transit.distanceLy<<' '<<s.transit.reservedFuelCells<<'\n';

    out<<"vehicles "<<state.vehicles.size()<<'\n';
    for(const auto& v:state.vehicles) {
        out<<"vehicle "<<v.stableId<<' '<<static_cast<int>(v.type)<<' '<<v.systemId<<' '<<v.planetId<<' ';writeAddress(out,v.address);
        out<<' '<<v.hull<<' '<<v.energy<<' '<<v.maintenance<<' '<<v.ownerStableId<<' '<<v.crewStableId<<' '<<v.autonomous<<' '<<v.hasRouteTarget<<' ';writeAddress(out,v.routeTarget);
        out<<' '<<v.cargo.size()<<'\n';
        for(const auto& c:v.cargo) out<<c.itemId<<' '<<c.count<<'\n';
    }

    out<<"orbitals "<<state.orbitalInfrastructure.size()<<'\n';
    for(const auto& o:state.orbitalInfrastructure) {
        out<<"orbital "<<o.stableId<<' '<<static_cast<int>(o.type)<<' '<<o.systemId<<' '<<o.planetId<<' '<<o.ownerStableId<<' '<<o.enabled<<' '<<o.powered<<' '
           <<o.hull<<' '<<o.maintenance<<' '<<o.processProgressSeconds<<' '<<o.cargo.size()<<' '<<o.serviceInventory.size()<<'\n';
        for(const auto& c:o.cargo) out<<c.itemId<<' '<<c.count<<'\n';
        for(const auto m:o.serviceInventory) out<<static_cast<int>(m)<<'\n';
    }
    out<<"END\n";
    return out.str();
}

std::optional<TravelPersistentState> deserializeTravelPersistentState(std::string_view text,std::string* error) {
    auto fail=[&](std::string e)->std::optional<TravelPersistentState>{if(error)*error=std::move(e);return std::nullopt;};
    std::istringstream in{std::string(text)};
    std::string tag; int version{};
    if(!(in>>tag>>version) || tag!="ELYSIUM_TRAVEL" || version!=static_cast<int>(TravelPersistentState::SchemaVersion)) return fail("unsupported travel schema");
    TravelPersistentState out;
    int kind{};
    if(!(in>>tag>>out.ship.stableId>>kind>>out.ship.location.systemId>>out.ship.location.planetId) || tag!="ship" || !validLocationKind(kind)) return fail("invalid ship header");
    out.ship.location.kind=static_cast<ShipLocationKind>(kind);
    if(!readAddress(in,out.ship.location.surfaceAddress)) return fail("invalid ship surface address");
    if(!(in>>out.ship.location.dockedObjectStableId>>out.ship.fuelCells>>out.ship.hull>>out.ship.maintenance>>out.ship.surfaceCargoLoaderStableId)) return fail("invalid ship scalar state");
    std::size_t count{};
    if(!(in>>tag>>count) || tag!="modules" || count>128) return fail("invalid module list");
    out.ship.installedModules.clear();
    for(std::size_t i=0;i<count;++i){int m{};if(!(in>>m)||m<=0||m>65535||!shipModuleDefinition(static_cast<ShipModuleId>(m)))return fail("invalid module id");out.ship.installedModules.push_back(static_cast<ShipModuleId>(m));}
    if(!(in>>tag>>count)||tag!="ship_cargo"||count>4096)return fail("invalid ship cargo count");
    for(std::size_t i=0;i<count;++i){ShipCargoStack c{};if(!(in>>c.itemId>>c.count))return fail("invalid ship cargo record");out.ship.cargo.push_back(c);}
    int active{},phase{};
    if(!(in>>tag>>active>>phase>>out.ship.transit.elapsedSeconds>>out.ship.transit.durationSeconds>>out.ship.transit.targetSystemId>>out.ship.transit.targetPlanetId) || tag!="transit" || !validLocationKind(phase)) return fail("invalid transit header");
    out.ship.transit.active=active!=0;out.ship.transit.phase=static_cast<ShipLocationKind>(phase);
    if(!readAddress(in,out.ship.transit.targetSurfaceAddress)) return fail("invalid transit target address");
    if(!(in>>out.ship.transit.targetDockStableId>>out.ship.transit.distanceLy>>out.ship.transit.reservedFuelCells)) return fail("invalid transit state");

    if(!(in>>tag>>count)||tag!="vehicles"||count>100000)return fail("invalid vehicle count");
    out.vehicles.reserve(count);
    for(std::size_t i=0;i<count;++i){
        VehicleState v{};int t{},autonomous{},hasRoute{};std::size_t cargoCount{};
        if(!(in>>tag>>v.stableId>>t>>v.systemId>>v.planetId)||tag!="vehicle"||!validVehicleType(t))return fail("invalid vehicle header");
        v.type=static_cast<VehicleType>(t);if(!readAddress(in,v.address))return fail("invalid vehicle address");
        if(!(in>>v.hull>>v.energy>>v.maintenance>>v.ownerStableId>>v.crewStableId>>autonomous>>hasRoute))return fail("invalid vehicle state");
        v.autonomous=autonomous!=0;v.hasRouteTarget=hasRoute!=0;if(!readAddress(in,v.routeTarget))return fail("invalid vehicle route address");
        if(!(in>>cargoCount)||cargoCount>4096)return fail("invalid vehicle cargo count");
        for(std::size_t c=0;c<cargoCount;++c){VehicleCargoStack stack{};if(!(in>>stack.itemId>>stack.count))return fail("invalid vehicle cargo");v.cargo.push_back(stack);}out.vehicles.push_back(std::move(v));
    }

    if(!(in>>tag>>count)||tag!="orbitals"||count>100000)return fail("invalid orbital count");
    out.orbitalInfrastructure.reserve(count);
    for(std::size_t i=0;i<count;++i){
        OrbitalInfrastructureState o{};int t{},enabled{},powered{};std::size_t cargoCount{},moduleCount{};
        if(!(in>>tag>>o.stableId>>t>>o.systemId>>o.planetId>>o.ownerStableId>>enabled>>powered>>o.hull>>o.maintenance>>o.processProgressSeconds>>cargoCount>>moduleCount) || tag!="orbital" || !validOrbitalType(t) || cargoCount>4096 || moduleCount>128) return fail("invalid orbital header");
        o.type=static_cast<OrbitalInfrastructureType>(t);o.enabled=enabled!=0;o.powered=powered!=0;
        for(std::size_t c=0;c<cargoCount;++c){OrbitalCargoStack stack{};if(!(in>>stack.itemId>>stack.count))return fail("invalid orbital cargo");o.cargo.push_back(stack);}
        for(std::size_t m=0;m<moduleCount;++m){int id{};if(!(in>>id)||!shipModuleDefinition(static_cast<ShipModuleId>(id)))return fail("invalid drydock module");o.serviceInventory.push_back(static_cast<ShipModuleId>(id));}
        out.orbitalInfrastructure.push_back(std::move(o));
    }
    if(!(in>>tag)||tag!="END")return fail("missing END marker");
    std::string validation;
    if(!validateTravelPersistentState(out,&validation)) return fail(validation);
    return out;
}

} // namespace elysium
