// Intended function: Translate renderer-neutral tactical UI intents into stable squad/military command requests for authoritative commit.
#include "TacticalCommandBridge.hpp"
namespace elysium::integration {
std::uint64_t TacticalCommandIntentIndex::keyOf(const TacticalCommandIntent& v) noexcept { return static_cast<std::uint64_t>(v.intentId); }
bool TacticalCommandIntentIndex::upsert(TacticalCommandIntent v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TacticalCommandIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool TacticalCommandIntentIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TacticalCommandIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const TacticalCommandIntent* TacticalCommandIntentIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TacticalCommandIntent& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
