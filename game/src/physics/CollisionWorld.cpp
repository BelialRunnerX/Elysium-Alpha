// Intended function: Represent renderer-independent collision queries against voxel/world geometry, structures, vehicles, actors, and local gravity frames.
#include "CollisionWorld.hpp"
namespace elysium::physics {
std::uint64_t CollisionQueryIndex::keyOf(const CollisionQuery& v) noexcept { return static_cast<std::uint64_t>(v.queryId); }
bool CollisionQueryIndex::upsert(CollisionQuery v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CollisionQuery& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool CollisionQueryIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CollisionQuery& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const CollisionQuery* CollisionQueryIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CollisionQuery& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
