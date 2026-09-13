// Intended function: Plan procedural creature feeding, nesting, territory, pack behavior, fleeing, ambush, curiosity, and migration.
#include "CreatureBehavior.hpp"
namespace elysium::ai {
std::uint64_t CreatureDecisionTable::keyOf(const CreatureDecision& v) noexcept { return static_cast<std::uint64_t>(v.creatureId); }
bool CreatureDecisionTable::set(CreatureDecision v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CreatureDecision& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool CreatureDecisionTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CreatureDecision& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const CreatureDecision* CreatureDecisionTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CreatureDecision& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
