// Intended function: Maintain compact site-local historical event streams linked to Galactic Chronicle stable identities.
#include "SiteChronicle.hpp"
namespace elysium::history {
std::uint64_t SiteChronicleRecordRegistry::key(const SiteChronicleRecord& r) noexcept { return static_cast<std::uint64_t>(r.eventId); }
bool SiteChronicleRecordRegistry::publish(SiteChronicleRecord r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const SiteChronicleRecord& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool SiteChronicleRecordRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const SiteChronicleRecord& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const SiteChronicleRecord* SiteChronicleRecordRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const SiteChronicleRecord& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::history
