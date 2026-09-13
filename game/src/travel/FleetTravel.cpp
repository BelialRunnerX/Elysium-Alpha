// Intended function: Coordinate multi-ship fleet movement, formation cohesion, fuel budgets, route legs, and arrival synchronization.
#include "FleetTravel.hpp"
namespace elysium::travel {
std::uint64_t FleetTravelStateRegistry::key(const FleetTravelState& r) noexcept { return static_cast<std::uint64_t>(r.fleetId); }
bool FleetTravelStateRegistry::publish(FleetTravelState r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const FleetTravelState& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool FleetTravelStateRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const FleetTravelState& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const FleetTravelState* FleetTravelStateRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const FleetTravelState& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::travel
