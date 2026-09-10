// Intended function: Manage enter/exit/seat/control ownership, cargo access, repair/refuel, autopilot handoff, and safe dismount requests.
#include "VehicleInteraction.hpp"
namespace elysium::gameplay {
std::uint64_t VehicleInteractionStateStore::idOf(const VehicleInteractionState& v) noexcept { return static_cast<std::uint64_t>(v.vehicleId); }
bool VehicleInteractionStateStore::put(VehicleInteractionState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const VehicleInteractionState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool VehicleInteractionStateStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const VehicleInteractionState& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const VehicleInteractionState* VehicleInteractionStateStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const VehicleInteractionState& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::gameplay
