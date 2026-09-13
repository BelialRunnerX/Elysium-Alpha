// Intended function: Provide stable item definitions for resources, tools, gear, consumables, components, artifacts, and logistics policy.
#include "ItemCatalogue.hpp"
namespace elysium::content {
std::uint64_t ItemRecordRegistry::key(const ItemRecord& r) noexcept { return static_cast<std::uint64_t>(r.itemId); }
bool ItemRecordRegistry::publish(ItemRecord r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ItemRecord& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool ItemRecordRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ItemRecord& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const ItemRecord* ItemRecordRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ItemRecord& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::content
