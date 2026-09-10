// Intended function: Track item/tool/weapon/armor condition, damage causes, repairability, break thresholds, maintenance, and degradation modifiers.
#include "DurabilitySystem.hpp"
namespace elysium::inventory {
std::uint64_t DurabilityStateIndex::keyOf(const DurabilityState& v) noexcept { return static_cast<std::uint64_t>(v.itemId); }
bool DurabilityStateIndex::upsert(DurabilityState v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const DurabilityState& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool DurabilityStateIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const DurabilityState& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const DurabilityState* DurabilityStateIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const DurabilityState& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
