// Intended function: Compose deterministic dungeon room graphs with guaranteed connectivity, loops, locks, keys, hazards, and encounter pacing.
#include "DungeonGrammar.hpp"
namespace elysium::procedural {
std::uint64_t DungeonGrammarStateIndex::keyOf(const DungeonGrammarState& v) noexcept { return static_cast<std::uint64_t>(v.grammarId); }
bool DungeonGrammarStateIndex::upsert(DungeonGrammarState v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const DungeonGrammarState& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool DungeonGrammarStateIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const DungeonGrammarState& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const DungeonGrammarState* DungeonGrammarStateIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const DungeonGrammarState& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
