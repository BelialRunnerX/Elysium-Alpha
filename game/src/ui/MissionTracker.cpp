// Intended function: Project active mission/objective stages, progress, optional goals, timers, target markers, blockers, and rewards.
#include "MissionTracker.hpp"
namespace elysium::ui {
std::uint64_t MissionTrackerStateCollection::idOf(const MissionTrackerState& v) noexcept { return static_cast<std::uint64_t>(v.viewId); }
bool MissionTrackerStateCollection::store(MissionTrackerState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const MissionTrackerState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool MissionTrackerStateCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const MissionTrackerState& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const MissionTrackerState* MissionTrackerStateCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const MissionTrackerState& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
