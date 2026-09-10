// Intended function: Translate evaluated strategic trade into atomic local item/credit/ownership commit batches before settlement acknowledgement.
#include "TradeCommitBridge.hpp"
namespace elysium::integration {
std::uint64_t TradeCommitIntentIndex::keyOf(const TradeCommitIntent& v) noexcept { return static_cast<std::uint64_t>(v.intentId); }
bool TradeCommitIntentIndex::upsert(TradeCommitIntent v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TradeCommitIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool TradeCommitIntentIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TradeCommitIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const TradeCommitIntent* TradeCommitIntentIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TradeCommitIntent& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
