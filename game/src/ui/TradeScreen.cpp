// Intended function: Project market quotes, manifests, legality, ownership, tariffs, standing effects, settlement state, and transfer blockers.
#include "TradeScreen.hpp"
namespace elysium::ui {
std::uint64_t TradeViewStateTable::keyOf(const TradeViewState& v) noexcept { return static_cast<std::uint64_t>(v.viewId); }
bool TradeViewStateTable::set(TradeViewState v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TradeViewState& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool TradeViewStateTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TradeViewState& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const TradeViewState* TradeViewStateTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const TradeViewState& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
