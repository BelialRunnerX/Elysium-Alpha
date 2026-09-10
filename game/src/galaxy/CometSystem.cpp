#include "galaxy/CometSystem.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Track generated comet orbits, volatile resources, hazards, mining opportunities, and temporary mission windows.
bool CometSystemSystem::submit(const CometSystemCommand& c) { if(!c.subject) return false; auto&s=map_[c.subject]; s.revision=revision_++; s.subject=c.subject; s.owner=c.owner; s.target=c.target; s.strength=c.strength; s.mode=c.mode; s.active=true; return true; }
const CometSystemState* CometSystemSystem::find(std::uint64_t id) const { auto it=map_.find(id); return it==map_.end()?nullptr:&it->second; }
std::vector<CometSystemState> CometSystemSystem::states() const { std::vector<CometSystemState> v; v.reserve(map_.size()); for(const auto&[id,s]:map_) if(s.active) v.push_back(s); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.subject<b.subject;}); return v; }
bool CometSystemSystem::cancel(std::uint64_t id) { return map_.erase(id)!=0; }
void CometSystemSystem::clear() { map_.clear(); revision_=1; }
}
