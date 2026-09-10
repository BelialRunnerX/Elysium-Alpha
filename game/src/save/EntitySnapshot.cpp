// Intended function: Represent versioned stable-entity snapshots independent of transient ECS handles and renderer identities.
#include "EntitySnapshot.hpp"
namespace elysium::save {
std::uint64_t EntitySnapshotRecordRegistry::key(const EntitySnapshotRecord& r) noexcept { return static_cast<std::uint64_t>(r.stableId); }
bool EntitySnapshotRecordRegistry::publish(EntitySnapshotRecord r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const EntitySnapshotRecord& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool EntitySnapshotRecordRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const EntitySnapshotRecord& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const EntitySnapshotRecord* EntitySnapshotRecordRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const EntitySnapshotRecord& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::save
