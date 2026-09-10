// Intended function: Track artifact maker, materials, sites, owners, battles, repairs, inscriptions, and ownership transitions.
#include "ArtifactProvenance.hpp"
namespace elysium::history {
std::uint64_t ArtifactProvenanceRecordRegistry::key(const ArtifactProvenanceRecord& r) noexcept { return static_cast<std::uint64_t>(r.artifactId); }
bool ArtifactProvenanceRecordRegistry::publish(ArtifactProvenanceRecord r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ArtifactProvenanceRecord& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool ArtifactProvenanceRecordRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ArtifactProvenanceRecord& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const ArtifactProvenanceRecord* ArtifactProvenanceRecordRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ArtifactProvenanceRecord& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::history
