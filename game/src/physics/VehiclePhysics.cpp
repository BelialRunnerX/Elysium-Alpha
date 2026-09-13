// Intended function: Represent simplified vehicle traction, steering, acceleration, suspension, slope, water/hover, and collision response state.
#include "VehiclePhysics.hpp"
namespace elysium::physics {
std::uint64_t VehiclePhysicsStateIndex::keyOf(const VehiclePhysicsState& v) noexcept { return static_cast<std::uint64_t>(v.vehicleId); }
bool VehiclePhysicsStateIndex::upsert(VehiclePhysicsState v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const VehiclePhysicsState& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool VehiclePhysicsStateIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const VehiclePhysicsState& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const VehiclePhysicsState* VehiclePhysicsStateIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const VehiclePhysicsState& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
