// Intended function: Maintain bounded local market offers, demand signals, stock pressure, service fees, and deterministic refresh epochs.
#include "MarketSystem.hpp"
namespace elysium::economy {
std::uint64_t MarketOfferTable::keyOf(const MarketOffer& v) noexcept { return static_cast<std::uint64_t>(v.offerId); }
bool MarketOfferTable::set(MarketOffer v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const MarketOffer& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool MarketOfferTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const MarketOffer& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const MarketOffer* MarketOfferTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const MarketOffer& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
