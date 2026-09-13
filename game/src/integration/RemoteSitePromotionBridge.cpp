// Intended function: Promote compact remote-site population/items/vehicles/threats into bounded active-shard materialization and demote back to summaries.
#include "RemoteSitePromotionBridge.hpp"
namespace elysium::integration {
std::uint64_t SitePromotionIntentIndex::keyOf(const SitePromotionIntent& v) noexcept { return static_cast<std::uint64_t>(v.intentId); }
bool SitePromotionIntentIndex::upsert(SitePromotionIntent v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SitePromotionIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool SitePromotionIntentIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SitePromotionIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const SitePromotionIntent* SitePromotionIntentIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SitePromotionIntent& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
