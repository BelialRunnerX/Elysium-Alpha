// Intended function: Manage domestic animals, feed, water, shelter, breeding, products, health, slaughter, and population pressure.
#include "LivestockSystem.hpp"
namespace elysium::agriculture {
std::uint64_t LivestockStateStore::idOf(const LivestockState& v) noexcept { return static_cast<std::uint64_t>(v.animalId); }
bool LivestockStateStore::put(LivestockState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const LivestockState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool LivestockStateStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const LivestockState& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const LivestockState* LivestockStateStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const LivestockState& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::agriculture
