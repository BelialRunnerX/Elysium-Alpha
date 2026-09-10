#include "fortress/SpaceportSystem.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Track pads, hangars, customs, cargo, passenger flows, fuel, maintenance, traffic control, and emergency response.
bool SpaceportSystemSystem::submit(const SpaceportSystemCommand& c) { if(!c.subject) return false; auto&s=map_[c.subject]; s.revision=revision_++; s.subject=c.subject; s.owner=c.owner; s.target=c.target; s.strength=c.strength; s.mode=c.mode; s.active=true; return true; }
const SpaceportSystemState* SpaceportSystemSystem::find(std::uint64_t id) const { auto it=map_.find(id); return it==map_.end()?nullptr:&it->second; }
std::vector<SpaceportSystemState> SpaceportSystemSystem::states() const { std::vector<SpaceportSystemState> v; v.reserve(map_.size()); for(const auto&[id,s]:map_) if(s.active) v.push_back(s); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.subject<b.subject;}); return v; }
bool SpaceportSystemSystem::cancel(std::uint64_t id) { return map_.erase(id)!=0; }
void SpaceportSystemSystem::clear() { map_.clear(); revision_=1; }
}
