// Intended function: Manage warp charge, route commitment, interdiction risk, cooldown, and deterministic arrival state.
#include "WarpSystem.hpp"
namespace elysium::travel {
std::uint64_t WarpStateRegistry::key(const WarpState& r) noexcept { return static_cast<std::uint64_t>(r.shipId); }
bool WarpStateRegistry::publish(WarpState r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const WarpState& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool WarpStateRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const WarpState& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const WarpState* WarpStateRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const WarpState& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::travel
