#include "fortress/ArcologyProject.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Represent dense self-contained city megastructures with housing, industry, utilities, transit, services, and resilience.
bool ArcologyProjectSystem::submit(const ArcologyProjectCommand& c) { if(!c.subject) return false; auto&s=map_[c.subject]; s.revision=revision_++; s.subject=c.subject; s.owner=c.owner; s.target=c.target; s.strength=c.strength; s.mode=c.mode; s.active=true; return true; }
const ArcologyProjectState* ArcologyProjectSystem::find(std::uint64_t id) const { auto it=map_.find(id); return it==map_.end()?nullptr:&it->second; }
std::vector<ArcologyProjectState> ArcologyProjectSystem::states() const { std::vector<ArcologyProjectState> v; v.reserve(map_.size()); for(const auto&[id,s]:map_) if(s.active) v.push_back(s); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.subject<b.subject;}); return v; }
bool ArcologyProjectSystem::cancel(std::uint64_t id) { return map_.erase(id)!=0; }
void ArcologyProjectSystem::clear() { map_.clear(); revision_=1; }
}
