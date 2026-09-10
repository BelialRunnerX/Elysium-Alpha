// Intended function: Plan local wildlife roaming, feeding, fleeing, hunting, nesting, territorial behavior, and active-bubble demotion.
#include "WildlifeAi.hpp"
namespace elysium::wildlife {
std::uint64_t WildlifeStateStore::idOf(const WildlifeState& v) noexcept { return static_cast<std::uint64_t>(v.actorId); }
bool WildlifeStateStore::put(WildlifeState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const WildlifeState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool WildlifeStateStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const WildlifeState& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const WildlifeState* WildlifeStateStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const WildlifeState& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::wildlife
