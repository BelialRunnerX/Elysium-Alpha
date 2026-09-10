// Intended function: Coordinate strategic convoys with escorts, manifests, route risk, speed, cohesion, and loss/recovery states.
#include "ConvoySystem.hpp"
namespace elysium::travel {
std::uint64_t ConvoyStateRegistry::key(const ConvoyState& r) noexcept { return static_cast<std::uint64_t>(r.convoyId); }
bool ConvoyStateRegistry::publish(ConvoyState r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ConvoyState& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool ConvoyStateRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ConvoyState& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const ConvoyState* ConvoyStateRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ConvoyState& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::travel
