// Intended function: Provide bounded stable spatial-query requests for nearby actors/items/machines/sites without planet-global scans.
#include "SpatialQuery.hpp"
namespace elysium::physics {
std::uint64_t SpatialQueryRequestIndex::keyOf(const SpatialQueryRequest& v) noexcept { return static_cast<std::uint64_t>(v.queryId); }
bool SpatialQueryRequestIndex::upsert(SpatialQueryRequest v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SpatialQueryRequest& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool SpatialQueryRequestIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SpatialQueryRequest& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const SpatialQueryRequest* SpatialQueryRequestIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SpatialQueryRequest& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
