// Intended function: Choose citizen self-care, assigned work, emergency response, social activity, sleep, food, and recreation intentions.
#include "CitizenDecision.hpp"
namespace elysium::ai {
std::uint64_t CitizenDecisionStateTable::keyOf(const CitizenDecisionState& v) noexcept { return static_cast<std::uint64_t>(v.citizenId); }
bool CitizenDecisionStateTable::set(CitizenDecisionState v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CitizenDecisionState& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool CitizenDecisionStateTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CitizenDecisionState& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const CitizenDecisionState* CitizenDecisionStateTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CitizenDecisionState& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
