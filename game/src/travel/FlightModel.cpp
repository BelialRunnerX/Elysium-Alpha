// Intended function: Represent simplified deterministic atmospheric/orbital flight state for thrust, velocity, lift proxy, drag, and autopilot.
#include "FlightModel.hpp"
namespace elysium::travel {
std::uint64_t FlightStateRegistry::key(const FlightState& r) noexcept { return static_cast<std::uint64_t>(r.vehicleId); }
bool FlightStateRegistry::publish(FlightState r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const FlightState& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool FlightStateRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const FlightState& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const FlightState* FlightStateRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const FlightState& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::travel
