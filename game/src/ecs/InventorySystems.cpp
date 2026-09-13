// Intended function: Stage ECS inventory/container/equipment/reservation commands around stable item and owner identity.
#include "InventorySystems.hpp"
namespace elysium::ecs {
std::uint64_t InventoryCommandCollection::idOf(const InventoryCommand& v) noexcept { return static_cast<std::uint64_t>(v.commandId); }
bool InventoryCommandCollection::store(InventoryCommand v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const InventoryCommand& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool InventoryCommandCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const InventoryCommand& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const InventoryCommand* InventoryCommandCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const InventoryCommand& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
