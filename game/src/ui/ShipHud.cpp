// Intended function: Project ship hull, shields, power, heat, fuel, cargo, modules, navigation, docking, hazard, and warp readiness.
#include "ShipHud.hpp"
namespace elysium::ui {
std::uint64_t ShipHudStateTable::keyOf(const ShipHudState& v) noexcept { return static_cast<std::uint64_t>(v.shipId); }
bool ShipHudStateTable::set(ShipHudState v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ShipHudState& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool ShipHudStateTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ShipHudState& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const ShipHudState* ShipHudStateTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ShipHudState& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
