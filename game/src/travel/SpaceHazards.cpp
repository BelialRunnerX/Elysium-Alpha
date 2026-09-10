// Intended function: Track deterministic space hazard volumes such as radiation belts, debris, storms, anomalies, and interdiction zones.
#include "SpaceHazards.hpp"
namespace elysium::travel {
std::uint64_t SpaceHazardRegistry::key(const SpaceHazard& r) noexcept { return static_cast<std::uint64_t>(r.hazardId); }
bool SpaceHazardRegistry::publish(SpaceHazard r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const SpaceHazard& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool SpaceHazardRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const SpaceHazard& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const SpaceHazard* SpaceHazardRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const SpaceHazard& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::travel
