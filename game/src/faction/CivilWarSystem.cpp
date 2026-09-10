#include "faction/CivilWarSystem.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Represent internal faction conflict with sides, claims, forces, settlements, diplomacy, objectives, and postwar settlement.
bool CivilWarSystemSystem::submit(const CivilWarSystemCommand& c) { if(!c.subject) return false; auto&s=map_[c.subject]; s.revision=revision_++; s.subject=c.subject; s.owner=c.owner; s.target=c.target; s.strength=c.strength; s.mode=c.mode; s.active=true; return true; }
const CivilWarSystemState* CivilWarSystemSystem::find(std::uint64_t id) const { auto it=map_.find(id); return it==map_.end()?nullptr:&it->second; }
std::vector<CivilWarSystemState> CivilWarSystemSystem::states() const { std::vector<CivilWarSystemState> v; v.reserve(map_.size()); for(const auto&[id,s]:map_) if(s.active) v.push_back(s); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.subject<b.subject;}); return v; }
bool CivilWarSystemSystem::cancel(std::uint64_t id) { return map_.erase(id)!=0; }
void CivilWarSystemSystem::clear() { map_.clear(); revision_=1; }
}
