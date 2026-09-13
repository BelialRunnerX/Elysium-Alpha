// Intended function: Project recipes, stations, inputs, queues, quality, unlocks, blockers, and batch controls for embodied/fortress crafting.
#include "CraftingScreen.hpp"
namespace elysium::ui {
std::uint64_t CraftingViewStateCollection::idOf(const CraftingViewState& v) noexcept { return static_cast<std::uint64_t>(v.viewId); }
bool CraftingViewStateCollection::store(CraftingViewState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CraftingViewState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool CraftingViewStateCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CraftingViewState& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const CraftingViewState* CraftingViewStateCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CraftingViewState& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
