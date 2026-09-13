#include "strategy/ReliefMissionSystem.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Coordinate aid fleets for famine, disaster, plague, siege, refugees, and infrastructure collapse.
bool ReliefMissionSystemSystem::submit(const ReliefMissionSystemCommand& c) { if(!c.subject) return false; auto&s=map_[c.subject]; s.revision=revision_++; s.subject=c.subject; s.owner=c.owner; s.target=c.target; s.strength=c.strength; s.mode=c.mode; s.active=true; return true; }
const ReliefMissionSystemState* ReliefMissionSystemSystem::find(std::uint64_t id) const { auto it=map_.find(id); return it==map_.end()?nullptr:&it->second; }
std::vector<ReliefMissionSystemState> ReliefMissionSystemSystem::states() const { std::vector<ReliefMissionSystemState> v; v.reserve(map_.size()); for(const auto&[id,s]:map_) if(s.active) v.push_back(s); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.subject<b.subject;}); return v; }
bool ReliefMissionSystemSystem::cancel(std::uint64_t id) { return map_.erase(id)!=0; }
void ReliefMissionSystemSystem::clear() { map_.clear(); revision_=1; }
}
