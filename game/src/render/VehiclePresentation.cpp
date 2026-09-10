// Intended function: Project vehicles/ships with hull/module state, damage, lights, thrusters, cargo attachments, doors, and occupant markers.
#include "VehiclePresentation.hpp"
namespace elysium::render {
std::uint64_t VehicleRenderStateCollection::idOf(const VehicleRenderState& v) noexcept { return static_cast<std::uint64_t>(v.stableId); }
bool VehicleRenderStateCollection::store(VehicleRenderState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const VehicleRenderState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool VehicleRenderStateCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const VehicleRenderState& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const VehicleRenderState* VehicleRenderStateCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const VehicleRenderState& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
