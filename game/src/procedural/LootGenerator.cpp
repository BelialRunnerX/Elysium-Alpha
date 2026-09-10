// Intended function: Generate deterministic loot bundles from loot tables, level/tier, biome/faction, rarity budget, uniqueness, and provenance policy.
#include "LootGenerator.hpp"
namespace elysium::procedural {
std::uint64_t LootGenerationStateIndex::keyOf(const LootGenerationState& v) noexcept { return static_cast<std::uint64_t>(v.rollId); }
bool LootGenerationStateIndex::upsert(LootGenerationState v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const LootGenerationState& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool LootGenerationStateIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const LootGenerationState& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const LootGenerationState* LootGenerationStateIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const LootGenerationState& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
