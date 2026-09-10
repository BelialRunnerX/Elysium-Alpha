// Intended function: Project authoritative player vitals, equipment, hotbar, hazards, alerts, objectives, and interaction prompts into HUD state.
#include "HudState.hpp"
namespace elysium::ui {
std::uint64_t HudSnapshotRegistry::key(const HudSnapshot& r) noexcept { return static_cast<std::uint64_t>(r.playerId); }
bool HudSnapshotRegistry::publish(HudSnapshot r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const HudSnapshot& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool HudSnapshotRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const HudSnapshot& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const HudSnapshot* HudSnapshotRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const HudSnapshot& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::ui
