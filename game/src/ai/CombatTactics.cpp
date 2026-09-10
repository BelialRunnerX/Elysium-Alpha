// Intended function: Select cover, range bands, flanks, abilities, retreats, focus targets, and squad-role actions from tactical state.
#include "CombatTactics.hpp"
namespace elysium::ai {
std::uint64_t TacticalDecisionTable::keyOf(const TacticalDecision& v) noexcept { return static_cast<std::uint64_t>(v.decisionId); }
bool TacticalDecisionTable::set(TacticalDecision v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TacticalDecision& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool TacticalDecisionTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TacticalDecision& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const TacticalDecision* TacticalDecisionTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TacticalDecision& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
