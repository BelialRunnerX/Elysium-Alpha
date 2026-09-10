// Intended function: Generate deterministic contracts/missions from local needs, factions, threats, discoveries, economy, history, and player standing.
#include "QuestGenerator.hpp"
namespace elysium::procedural {
std::uint64_t QuestSeedIndex::keyOf(const QuestSeed& v) noexcept { return static_cast<std::uint64_t>(v.questId); }
bool QuestSeedIndex::upsert(QuestSeed v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const QuestSeed& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool QuestSeedIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const QuestSeed& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const QuestSeed* QuestSeedIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const QuestSeed& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
