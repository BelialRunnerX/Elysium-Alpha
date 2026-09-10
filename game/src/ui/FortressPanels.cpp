// Intended function: Project citizen/jobs/stocks/zones/rooms/medicine/military/justice/governance/trade/history panels from authoritative snapshots.
#include "FortressPanels.hpp"
namespace elysium::ui {
std::uint64_t FortressPanelStateCollection::idOf(const FortressPanelState& v) noexcept { return static_cast<std::uint64_t>(v.panelId); }
bool FortressPanelStateCollection::store(FortressPanelState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const FortressPanelState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool FortressPanelStateCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const FortressPanelState& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const FortressPanelState* FortressPanelStateCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const FortressPanelState& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
