// Intended function: Track projectile position/velocity/gravity/drag/penetration/lifetime and deterministic collision sequence state.
#include "ProjectilePhysics.hpp"
namespace elysium::physics {
std::uint64_t ProjectilePhysicsStateIndex::keyOf(const ProjectilePhysicsState& v) noexcept { return static_cast<std::uint64_t>(v.projectileId); }
bool ProjectilePhysicsStateIndex::upsert(ProjectilePhysicsState v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ProjectilePhysicsState& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool ProjectilePhysicsStateIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ProjectilePhysicsState& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const ProjectilePhysicsState* ProjectilePhysicsStateIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ProjectilePhysicsState& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
