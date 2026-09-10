// Intended function: Project vehicle condition, energy/fuel, cargo, seats, terrain/navigation, modules, hazards, and active control mode.
#include "VehicleHud.hpp"
namespace elysium::ui {
std::uint64_t VehicleHudStateTable::keyOf(const VehicleHudState& v) noexcept { return static_cast<std::uint64_t>(v.vehicleId); }
bool VehicleHudStateTable::set(VehicleHudState v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const VehicleHudState& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool VehicleHudStateTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const VehicleHudState& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const VehicleHudState* VehicleHudStateTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const VehicleHudState& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
