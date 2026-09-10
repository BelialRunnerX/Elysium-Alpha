// Intended function: Translate machine/local-network inventory batches into physical-item ledger promotion/reservation/delivery requests when identity is required.
#include "IndustryItemBridge.hpp"
namespace elysium::integration {
std::uint64_t IndustryItemIntentIndex::keyOf(const IndustryItemIntent& v) noexcept { return static_cast<std::uint64_t>(v.intentId); }
bool IndustryItemIntentIndex::upsert(IndustryItemIntent v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const IndustryItemIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool IndustryItemIntentIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const IndustryItemIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const IndustryItemIntent* IndustryItemIntentIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const IndustryItemIntent& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
