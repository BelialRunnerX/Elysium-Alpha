// Intended function: Provide research definitions with prerequisites, costs, disciplines, outputs, and Chronicle significance.
#include "ResearchCatalogue.hpp"
namespace elysium::content {
std::uint64_t ResearchRecordRegistry::key(const ResearchRecord& r) noexcept { return static_cast<std::uint64_t>(r.researchId); }
bool ResearchRecordRegistry::publish(ResearchRecord r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ResearchRecord& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool ResearchRecordRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ResearchRecord& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const ResearchRecord* ResearchRecordRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ResearchRecord& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::content
