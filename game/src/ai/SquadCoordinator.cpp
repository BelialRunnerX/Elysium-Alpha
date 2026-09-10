// Intended function: Coordinate squad formation, role assignment, focus targets, suppression, retreat, rally, breach, and evacuation behavior.
#include "SquadCoordinator.hpp"
namespace elysium::ai {
std::uint64_t SquadDecisionTable::keyOf(const SquadDecision& v) noexcept { return static_cast<std::uint64_t>(v.squadId); }
bool SquadDecisionTable::set(SquadDecision v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SquadDecision& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool SquadDecisionTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SquadDecision& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const SquadDecision* SquadDecisionTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SquadDecision& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
