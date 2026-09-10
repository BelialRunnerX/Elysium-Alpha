// Intended function: Translate committed actor deaths/destruction into deterministic loot-roll, provenance, salvage, and history requests.
#include "CombatLootBridge.hpp"
namespace elysium::integration {
std::uint64_t CombatLootIntentIndex::keyOf(const CombatLootIntent& v) noexcept { return static_cast<std::uint64_t>(v.intentId); }
bool CombatLootIntentIndex::upsert(CombatLootIntent v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CombatLootIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool CombatLootIntentIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CombatLootIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const CombatLootIntent* CombatLootIntentIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CombatLootIntent& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
