// Intended function: Represent future session authority ownership, command sequence windows, reconciliation ticks, and migration state.
#include "SessionAuthority.hpp"
namespace elysium::network {
std::uint64_t AuthorityRecordRegistry::key(const AuthorityRecord& r) noexcept { return static_cast<std::uint64_t>(r.stableId); }
bool AuthorityRecordRegistry::publish(AuthorityRecord r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const AuthorityRecord& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool AuthorityRecordRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const AuthorityRecord& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const AuthorityRecord* AuthorityRecordRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const AuthorityRecord& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::network
