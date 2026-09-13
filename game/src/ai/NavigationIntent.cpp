// Intended function: Represent actor movement goals, locomotion class, route policy, hazard tolerance, urgency, and cancellation token.
#include "NavigationIntent.hpp"
namespace elysium::ai {
std::uint64_t NavigationIntentStateTable::keyOf(const NavigationIntentState& v) noexcept { return static_cast<std::uint64_t>(v.intentId); }
bool NavigationIntentStateTable::set(NavigationIntentState v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const NavigationIntentState& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool NavigationIntentStateTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const NavigationIntentState& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const NavigationIntentState* NavigationIntentStateTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const NavigationIntentState& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
