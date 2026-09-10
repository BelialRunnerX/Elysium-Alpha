// Intended function: Provide stable structure/blueprint definitions for habitat, industry, defense, utilities, civic, and orbital construction.
#include "StructureCatalogue.hpp"
namespace elysium::content {
std::uint64_t StructureRecordRegistry::key(const StructureRecord& r) noexcept { return static_cast<std::uint64_t>(r.structureId); }
bool StructureRecordRegistry::publish(StructureRecord r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const StructureRecord& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool StructureRecordRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const StructureRecord& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const StructureRecord* StructureRecordRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const StructureRecord& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::content
