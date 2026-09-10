// Intended function: Translate committed simulation outcomes into Chronicle-worthy stable event envelopes without making history authoritative over gameplay.
#include "ChronicleEventBridge.hpp"
namespace elysium::integration {
std::uint64_t ChroniclePublishIntentIndex::keyOf(const ChroniclePublishIntent& v) noexcept { return static_cast<std::uint64_t>(v.intentId); }
bool ChroniclePublishIntentIndex::upsert(ChroniclePublishIntent v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ChroniclePublishIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool ChroniclePublishIntentIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ChroniclePublishIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const ChroniclePublishIntent* ChroniclePublishIntentIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ChroniclePublishIntent& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
