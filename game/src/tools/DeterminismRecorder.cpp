// Intended function: Record deterministic input/output hashes, named seed streams, command ordering, and replay checkpoints for debugging later.
#include "DeterminismRecorder.hpp"
namespace elysium::tools {
std::uint64_t DeterminismCheckpointRegistry::key(const DeterminismCheckpoint& r) noexcept { return static_cast<std::uint64_t>(r.checkpointId); }
bool DeterminismCheckpointRegistry::publish(DeterminismCheckpoint r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const DeterminismCheckpoint& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool DeterminismCheckpointRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const DeterminismCheckpoint& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const DeterminismCheckpoint* DeterminismCheckpointRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const DeterminismCheckpoint& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::tools
