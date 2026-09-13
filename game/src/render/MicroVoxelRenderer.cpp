// Intended function: Represent refined 16^3 microvoxel draw packets, surface masks, damage layers, and distance-based simplification.
#include "MicroVoxelRenderer.hpp"
namespace elysium::render {
std::uint64_t MicroVoxelDrawRecordRegistry::key(const MicroVoxelDrawRecord& r) noexcept { return static_cast<std::uint64_t>(r.recordId); }
bool MicroVoxelDrawRecordRegistry::publish(MicroVoxelDrawRecord r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const MicroVoxelDrawRecord& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool MicroVoxelDrawRecordRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const MicroVoxelDrawRecord& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const MicroVoxelDrawRecord* MicroVoxelDrawRecordRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const MicroVoxelDrawRecord& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::render
