#include "strategy/FleetProductionPlanner.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Allocate strategic shipbuilding capacity across hull classes, repairs, refits, escorts, logistics, and reserve goals.
bool FleetProductionPlannerSystem::submit(const FleetProductionPlannerCommand& c) { if(!c.subject) return false; auto&s=map_[c.subject]; s.revision=revision_++; s.subject=c.subject; s.owner=c.owner; s.target=c.target; s.strength=c.strength; s.mode=c.mode; s.active=true; return true; }
const FleetProductionPlannerState* FleetProductionPlannerSystem::find(std::uint64_t id) const { auto it=map_.find(id); return it==map_.end()?nullptr:&it->second; }
std::vector<FleetProductionPlannerState> FleetProductionPlannerSystem::states() const { std::vector<FleetProductionPlannerState> v; v.reserve(map_.size()); for(const auto&[id,s]:map_) if(s.active) v.push_back(s); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.subject<b.subject;}); return v; }
bool FleetProductionPlannerSystem::cancel(std::uint64_t id) { return map_.erase(id)!=0; }
void FleetProductionPlannerSystem::clear() { map_.clear(); revision_=1; }
}
