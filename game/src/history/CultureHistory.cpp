// Intended function: Track cultural works, traditions, institutions, migrations, language shifts, and historical influence.
#include "CultureHistory.hpp"
namespace elysium::history {
std::uint64_t CultureHistoryRecordRegistry::key(const CultureHistoryRecord& r) noexcept { return static_cast<std::uint64_t>(r.cultureId); }
bool CultureHistoryRecordRegistry::publish(CultureHistoryRecord r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const CultureHistoryRecord& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool CultureHistoryRecordRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const CultureHistoryRecord& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const CultureHistoryRecord* CultureHistoryRecordRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const CultureHistoryRecord& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::history
