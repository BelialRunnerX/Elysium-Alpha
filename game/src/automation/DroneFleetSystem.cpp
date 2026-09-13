#include "automation/DroneFleetSystem.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Track stable drone fleets, roles, batteries, payloads, maintenance, dispatch, recall, and remote aggregate state.
bool DroneFleetSystemSystem::submit(const DroneFleetSystemCommand& c) { if(!c.subject) return false; auto&s=map_[c.subject]; s.revision=revision_++; s.subject=c.subject; s.owner=c.owner; s.target=c.target; s.strength=c.strength; s.mode=c.mode; s.active=true; return true; }
const DroneFleetSystemState* DroneFleetSystemSystem::find(std::uint64_t id) const { auto it=map_.find(id); return it==map_.end()?nullptr:&it->second; }
std::vector<DroneFleetSystemState> DroneFleetSystemSystem::states() const { std::vector<DroneFleetSystemState> v; v.reserve(map_.size()); for(const auto&[id,s]:map_) if(s.active) v.push_back(s); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.subject<b.subject;}); return v; }
bool DroneFleetSystemSystem::cancel(std::uint64_t id) { return map_.erase(id)!=0; }
void DroneFleetSystemSystem::clear() { map_.clear(); revision_=1; }
}
