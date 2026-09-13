// Intended function: Track installed vehicle modules, slots, condition, power draw, mass, and enabled state.
#include "VehicleModules.hpp"
namespace elysium::travel {
std::uint64_t VehicleModuleStateRegistry::key(const VehicleModuleState& r) noexcept { return static_cast<std::uint64_t>(r.vehicleId); }
bool VehicleModuleStateRegistry::publish(VehicleModuleState r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const VehicleModuleState& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool VehicleModuleStateRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const VehicleModuleState& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const VehicleModuleState* VehicleModuleStateRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const VehicleModuleState& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::travel
