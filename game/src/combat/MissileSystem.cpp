#include "combat/MissileSystem.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Track guided missile target locks, fuel, seeker state, countermeasures, interception, proximity detonation, and warheads.
bool MissileSystemSystem::submit(const MissileSystemCommand& c) { if(!c.subject) return false; auto&s=map_[c.subject]; s.revision=revision_++; s.subject=c.subject; s.owner=c.owner; s.target=c.target; s.strength=c.strength; s.mode=c.mode; s.active=true; return true; }
const MissileSystemState* MissileSystemSystem::find(std::uint64_t id) const { auto it=map_.find(id); return it==map_.end()?nullptr:&it->second; }
std::vector<MissileSystemState> MissileSystemSystem::states() const { std::vector<MissileSystemState> v; v.reserve(map_.size()); for(const auto&[id,s]:map_) if(s.active) v.push_back(s); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.subject<b.subject;}); return v; }
bool MissileSystemSystem::cancel(std::uint64_t id) { return map_.erase(id)!=0; }
void MissileSystemSystem::clear() { map_.clear(); revision_=1; }
}
