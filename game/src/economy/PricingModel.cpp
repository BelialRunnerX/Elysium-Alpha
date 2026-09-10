// Intended function: Calculate deterministic local buy/sell price bands from base value, supply, demand, scarcity, risk, standing, and tariffs.
#include "PricingModel.hpp"
namespace elysium::economy {
std::uint64_t PriceQuoteTable::keyOf(const PriceQuote& v) noexcept { return static_cast<std::uint64_t>(v.quoteId); }
bool PriceQuoteTable::set(PriceQuote v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const PriceQuote& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool PriceQuoteTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const PriceQuote& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const PriceQuote* PriceQuoteTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const PriceQuote& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
