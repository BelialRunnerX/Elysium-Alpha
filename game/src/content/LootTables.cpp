// Intended function: Provide deterministic weighted loot-table entries with tier gates, quantity ranges, uniqueness, and provenance policies.
#include "LootTables.hpp"
namespace elysium::content {
std::uint64_t LootTableEntryRegistry::key(const LootTableEntry& r) noexcept { return static_cast<std::uint64_t>(r.entryId); }
bool LootTableEntryRegistry::publish(LootTableEntry r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const LootTableEntry& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool LootTableEntryRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const LootTableEntry& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const LootTableEntry* LootTableEntryRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const LootTableEntry& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::content
