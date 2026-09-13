#include "economy/SalvageMarket.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Price wrecks, scrap, components, artifacts, claims, towing, hazards, and recovered modules from market context.
bool SalvageMarketSystem::submit(const SalvageMarketCommand& c) { if(!c.subject) return false; auto&s=map_[c.subject]; s.revision=revision_++; s.subject=c.subject; s.owner=c.owner; s.target=c.target; s.strength=c.strength; s.mode=c.mode; s.active=true; return true; }
const SalvageMarketState* SalvageMarketSystem::find(std::uint64_t id) const { auto it=map_.find(id); return it==map_.end()?nullptr:&it->second; }
std::vector<SalvageMarketState> SalvageMarketSystem::states() const { std::vector<SalvageMarketState> v; v.reserve(map_.size()); for(const auto&[id,s]:map_) if(s.active) v.push_back(s); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.subject<b.subject;}); return v; }
bool SalvageMarketSystem::cancel(std::uint64_t id) { return map_.erase(id)!=0; }
void SalvageMarketSystem::clear() { map_.clear(); revision_=1; }
}
