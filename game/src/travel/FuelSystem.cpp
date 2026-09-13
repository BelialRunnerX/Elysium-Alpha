// Intended function: Track ship/vehicle fuel tanks, reserve policy, consumption rates, refuel intents, and stranded-state diagnostics.
#include "FuelSystem.hpp"
namespace elysium::travel {
std::uint64_t FuelStateRegistry::key(const FuelState& r) noexcept { return static_cast<std::uint64_t>(r.ownerId); }
bool FuelStateRegistry::publish(FuelState r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const FuelState& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool FuelStateRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const FuelState& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const FuelState* FuelStateRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const FuelState& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::travel
