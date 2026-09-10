// Intended function: Encode versioned sparse chunk edits, tombstones, object deltas, checksums, and compatibility metadata.
#include "ChunkDeltaCodec.hpp"
namespace elysium::save {
std::uint64_t ChunkDeltaRecordRegistry::key(const ChunkDeltaRecord& r) noexcept { return static_cast<std::uint64_t>(r.recordId); }
bool ChunkDeltaRecordRegistry::publish(ChunkDeltaRecord r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ChunkDeltaRecord& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool ChunkDeltaRecordRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ChunkDeltaRecord& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const ChunkDeltaRecord* ChunkDeltaRecordRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ChunkDeltaRecord& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::save
