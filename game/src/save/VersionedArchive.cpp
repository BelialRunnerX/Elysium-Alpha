// Intended function: Track archive sections, schema versions, generator fingerprints, migration requirements, and integrity digests.
#include "VersionedArchive.hpp"
namespace elysium::save {
std::uint64_t ArchiveSectionRegistry::key(const ArchiveSection& r) noexcept { return static_cast<std::uint64_t>(r.sectionId); }
bool ArchiveSectionRegistry::publish(ArchiveSection r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ArchiveSection& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool ArchiveSectionRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ArchiveSection& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const ArchiveSection* ArchiveSectionRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ArchiveSection& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::save
