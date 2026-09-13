// Intended function: Represent immutable component snapshot envelopes for persistence, replication, inspection, and shard transfer.
#include "ComponentSnapshots.hpp"
namespace elysium::ecs {
std::uint64_t ComponentSnapshotCollection::idOf(const ComponentSnapshot& v) noexcept { return static_cast<std::uint64_t>(v.snapshotId); }
bool ComponentSnapshotCollection::store(ComponentSnapshot v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ComponentSnapshot& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool ComponentSnapshotCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ComponentSnapshot& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const ComponentSnapshot* ComponentSnapshotCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ComponentSnapshot& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
