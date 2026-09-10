// Intended function: Translate mission stage/completion/failure outcomes into Chronicle events and persistent objective consequences.
#include "MissionHistoryBridge.hpp"
namespace elysium::integration {
std::uint64_t MissionHistoryIntentIndex::keyOf(const MissionHistoryIntent& v) noexcept { return static_cast<std::uint64_t>(v.intentId); }
bool MissionHistoryIntentIndex::upsert(MissionHistoryIntent v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const MissionHistoryIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool MissionHistoryIntentIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const MissionHistoryIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const MissionHistoryIntent* MissionHistoryIntentIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const MissionHistoryIntent& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
