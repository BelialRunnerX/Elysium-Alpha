#include "ui/DroneScreenModel.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Build drone fleet roles, battery, payload, assignments, maintenance, failures, and automation command projections.
bool DroneScreenModelSystem::submit(const DroneScreenModelCommand& c) { if(!c.subject) return false; auto&s=map_[c.subject]; s.revision=revision_++; s.subject=c.subject; s.owner=c.owner; s.target=c.target; s.strength=c.strength; s.mode=c.mode; s.active=true; return true; }
const DroneScreenModelState* DroneScreenModelSystem::find(std::uint64_t id) const { auto it=map_.find(id); return it==map_.end()?nullptr:&it->second; }
std::vector<DroneScreenModelState> DroneScreenModelSystem::states() const { std::vector<DroneScreenModelState> v; v.reserve(map_.size()); for(const auto&[id,s]:map_) if(s.active) v.push_back(s); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.subject<b.subject;}); return v; }
bool DroneScreenModelSystem::cancel(std::uint64_t id) { return map_.erase(id)!=0; }
void DroneScreenModelSystem::clear() { map_.clear(); revision_=1; }
}
