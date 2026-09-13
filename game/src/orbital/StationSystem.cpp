// Intended function: Manage orbital station modules, docking, power, atmosphere, cargo, population, defenses, and expansion anchors.
#include "StationSystem.hpp"
namespace elysium::orbital {
std::uint64_t StationStateStore::idOf(const StationState& v) noexcept { return static_cast<std::uint64_t>(v.stationId); }
bool StationStateStore::put(StationState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const StationState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool StationStateStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const StationState& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const StationState* StationStateStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const StationState& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::orbital
