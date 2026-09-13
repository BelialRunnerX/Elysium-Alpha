// Intended function: Track conservative occlusion candidates and visibility decisions without making render state authoritative gameplay truth.
#include "OcclusionPlanner.hpp"
namespace elysium::render {
std::uint64_t OcclusionCandidateRegistry::key(const OcclusionCandidate& r) noexcept { return static_cast<std::uint64_t>(r.objectId); }
bool OcclusionCandidateRegistry::publish(OcclusionCandidate r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const OcclusionCandidate& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool OcclusionCandidateRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const OcclusionCandidate& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const OcclusionCandidate* OcclusionCandidateRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const OcclusionCandidate& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::render
