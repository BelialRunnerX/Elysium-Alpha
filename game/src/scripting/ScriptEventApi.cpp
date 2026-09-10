// Intended function: Expose read-only event subscriptions for combat, jobs, discoveries, trade, history, weather, machines, and player actions.
#include "ScriptEventApi.hpp"
namespace elysium::scripting {
std::uint64_t ScriptSubscriptionCollection::idOf(const ScriptSubscription& v) noexcept { return static_cast<std::uint64_t>(v.subscriptionId); }
bool ScriptSubscriptionCollection::store(ScriptSubscription v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ScriptSubscription& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool ScriptSubscriptionCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ScriptSubscription& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const ScriptSubscription* ScriptSubscriptionCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ScriptSubscription& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
