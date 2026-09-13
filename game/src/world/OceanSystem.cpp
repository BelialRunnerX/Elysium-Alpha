// Intended function: Represent ocean/lake water bodies, sea level, currents, salinity, contamination, freezing, and resource zones.
#include "OceanSystem.hpp"
namespace elysium::world {
std::uint64_t WaterBodyStateStore::idOf(const WaterBodyState& v) noexcept { return static_cast<std::uint64_t>(v.waterBodyId); }
bool WaterBodyStateStore::put(WaterBodyState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const WaterBodyState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool WaterBodyStateStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const WaterBodyState& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const WaterBodyState* WaterBodyStateStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const WaterBodyState& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::world
