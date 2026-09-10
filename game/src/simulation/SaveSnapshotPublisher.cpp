// Intended function: Publish immutable versioned snapshots after Commit for asynchronous persistence without exposing live mutable state.
#include "SaveSnapshotPublisher.hpp"
namespace elysium::simulation {
std::uint64_t SnapshotEnvelopeTable::keyOf(const SnapshotEnvelope& v) noexcept { return static_cast<std::uint64_t>(v.snapshotId); }
bool SnapshotEnvelopeTable::set(SnapshotEnvelope v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SnapshotEnvelope& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool SnapshotEnvelopeTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SnapshotEnvelope& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const SnapshotEnvelope* SnapshotEnvelopeTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SnapshotEnvelope& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
