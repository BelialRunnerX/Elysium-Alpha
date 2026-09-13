// Intended function: Represent mod/content-pack manifests, dependencies, stable namespaces, schema requirements, and load order.
#include "ContentPackLoader.hpp"
namespace elysium::mod {
std::uint64_t ContentPackRecordRegistry::key(const ContentPackRecord& r) noexcept { return static_cast<std::uint64_t>(r.packId); }
bool ContentPackRecordRegistry::publish(ContentPackRecord r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ContentPackRecord& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool ContentPackRecordRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ContentPackRecord& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const ContentPackRecord* ContentPackRecordRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ContentPackRecord& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::mod
