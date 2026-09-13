#include "gameplay/CyberneticImplants.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Track installed cybernetics, body slots, power, heat, maintenance, compatibility, bonuses, and failure states.
bool CyberneticImplantsSystem::submit(const CyberneticImplantsCommand& c) { if(!c.subject) return false; auto&s=map_[c.subject]; s.revision=revision_++; s.subject=c.subject; s.owner=c.owner; s.target=c.target; s.strength=c.strength; s.mode=c.mode; s.active=true; return true; }
const CyberneticImplantsState* CyberneticImplantsSystem::find(std::uint64_t id) const { auto it=map_.find(id); return it==map_.end()?nullptr:&it->second; }
std::vector<CyberneticImplantsState> CyberneticImplantsSystem::states() const { std::vector<CyberneticImplantsState> v; v.reserve(map_.size()); for(const auto&[id,s]:map_) if(s.active) v.push_back(s); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.subject<b.subject;}); return v; }
bool CyberneticImplantsSystem::cancel(std::uint64_t id) { return map_.erase(id)!=0; }
void CyberneticImplantsSystem::clear() { map_.clear(); revision_=1; }
}
