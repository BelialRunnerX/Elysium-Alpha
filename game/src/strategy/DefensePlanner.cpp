#include "strategy/DefensePlanner.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Generate system/planet defense posture from threats, sensors, fleets, fortifications, logistics, and evacuation capacity.
bool DefensePlannerSystem::submit(const DefensePlannerCommand& c) { if(!c.subject) return false; auto&s=map_[c.subject]; s.revision=revision_++; s.subject=c.subject; s.owner=c.owner; s.target=c.target; s.strength=c.strength; s.mode=c.mode; s.active=true; return true; }
const DefensePlannerState* DefensePlannerSystem::find(std::uint64_t id) const { auto it=map_.find(id); return it==map_.end()?nullptr:&it->second; }
std::vector<DefensePlannerState> DefensePlannerSystem::states() const { std::vector<DefensePlannerState> v; v.reserve(map_.size()); for(const auto&[id,s]:map_) if(s.active) v.push_back(s); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.subject<b.subject;}); return v; }
bool DefensePlannerSystem::cancel(std::uint64_t id) { return map_.erase(id)!=0; }
void DefensePlannerSystem::clear() { map_.clear(); revision_=1; }
}
