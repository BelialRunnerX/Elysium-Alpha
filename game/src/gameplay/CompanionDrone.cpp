// Intended function: Track personal drone follow/hold/scout/mine/repair/haul modes, energy, inventory, damage, and recall behavior.
#include "CompanionDrone.hpp"
namespace elysium::gameplay {
std::uint64_t CompanionDroneStateStore::idOf(const CompanionDroneState& v) noexcept { return static_cast<std::uint64_t>(v.droneId); }
bool CompanionDroneStateStore::put(CompanionDroneState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const CompanionDroneState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool CompanionDroneStateStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const CompanionDroneState& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const CompanionDroneState* CompanionDroneStateStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const CompanionDroneState& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::gameplay
