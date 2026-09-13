// Intended function: Represent fortress stockpile filters, priorities, capacity targets, hazard rules, and give/take links.
#include "StockpilePolicy.hpp"
namespace elysium::fortress {
std::uint64_t StockpilePolicyStateStore::idOf(const StockpilePolicyState& v) noexcept { return static_cast<std::uint64_t>(v.stockpileId); }
bool StockpilePolicyStateStore::put(StockpilePolicyState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const StockpilePolicyState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool StockpilePolicyStateStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const StockpilePolicyState& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const StockpilePolicyState* StockpilePolicyStateStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const StockpilePolicyState& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::fortress
