// Intended function: Queue deterministic world/actor raycasts for mining, building, weapons, scanning, interactions, sensors, and AI line-of-sight.
#include "RaycastSystem.hpp"
namespace elysium::physics {
std::uint64_t RaycastQueryIndex::keyOf(const RaycastQuery& v) noexcept { return static_cast<std::uint64_t>(v.queryId); }
bool RaycastQueryIndex::upsert(RaycastQuery v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const RaycastQuery& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool RaycastQueryIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const RaycastQuery& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const RaycastQuery* RaycastQueryIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const RaycastQuery& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
