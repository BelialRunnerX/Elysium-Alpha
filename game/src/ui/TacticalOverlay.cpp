// Intended function: Project squads, hostiles, sensors, defenses, evacuation zones, breaches, hazards, and tactical orders into overlay state.
#include "TacticalOverlay.hpp"
namespace elysium::ui {
std::uint64_t TacticalMarkerTable::keyOf(const TacticalMarker& v) noexcept { return static_cast<std::uint64_t>(v.markerId); }
bool TacticalMarkerTable::set(TacticalMarker v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TacticalMarker& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool TacticalMarkerTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TacticalMarker& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const TacticalMarker* TacticalMarkerTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TacticalMarker& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
