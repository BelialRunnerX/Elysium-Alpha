// Intended function: Project player/container inventories, filters, stacks, stable items, equipment, hotbar, encumbrance, and transfer actions.
#include "InventoryScreen.hpp"
namespace elysium::ui {
std::uint64_t InventoryViewStateCollection::idOf(const InventoryViewState& v) noexcept { return static_cast<std::uint64_t>(v.viewId); }
bool InventoryViewStateCollection::store(InventoryViewState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const InventoryViewState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool InventoryViewStateCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const InventoryViewState& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const InventoryViewState* InventoryViewStateCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const InventoryViewState& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
