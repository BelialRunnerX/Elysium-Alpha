// Intended function: Apply trade inspection/smuggling/contract standing deltas to system-local Favor/Suspicion without making economy own faction state.
#include "StandingTradeBridge.hpp"
namespace elysium::integration {
std::uint64_t StandingDeltaIntentIndex::keyOf(const StandingDeltaIntent& v) noexcept { return static_cast<std::uint64_t>(v.intentId); }
bool StandingDeltaIntentIndex::upsert(StandingDeltaIntent v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const StandingDeltaIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool StandingDeltaIntentIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const StandingDeltaIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const StandingDeltaIntent* StandingDeltaIntentIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const StandingDeltaIntent& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
