// Intended function: Schedule chunk mesh builds/rebuilds by LOD, visibility, edit urgency, worker budget, and stale-revision rejection.
#include "LodMeshScheduler.hpp"
namespace elysium::render {
std::uint64_t MeshBuildRequestRegistry::key(const MeshBuildRequest& r) noexcept { return static_cast<std::uint64_t>(r.requestId); }
bool MeshBuildRequestRegistry::publish(MeshBuildRequest r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const MeshBuildRequest& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool MeshBuildRequestRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const MeshBuildRequest& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const MeshBuildRequest* MeshBuildRequestRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const MeshBuildRequest& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::render
