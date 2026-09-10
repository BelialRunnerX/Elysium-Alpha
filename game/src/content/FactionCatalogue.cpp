// Intended function: Provide stable faction definitions, ideology tags, default relations, law policies, visuals, and economic archetypes.
#include "FactionCatalogue.hpp"
namespace elysium::content {
std::uint64_t FactionRecordRegistry::key(const FactionRecord& r) noexcept { return static_cast<std::uint64_t>(r.factionId); }
bool FactionRecordRegistry::publish(FactionRecord r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const FactionRecord& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool FactionRecordRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const FactionRecord& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const FactionRecord* FactionRecordRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const FactionRecord& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::content
