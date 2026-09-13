// Intended function: Expose stable read-only world/entity/object snapshots and reason strings for developer inspection tooling.
#include "WorldInspector.hpp"
namespace elysium::tools {
std::uint64_t InspectionRecordRegistry::key(const InspectionRecord& r) noexcept { return static_cast<std::uint64_t>(r.recordId); }
bool InspectionRecordRegistry::publish(InspectionRecord r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const InspectionRecord& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool InspectionRecordRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const InspectionRecord& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const InspectionRecord* InspectionRecordRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const InspectionRecord& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::tools
