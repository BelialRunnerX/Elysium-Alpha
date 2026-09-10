// Intended function: Track save slots, world seeds, display names, timestamps, schema/generator identity, thumbnails, and compatibility state.
#include "SaveSlotManager.hpp"
namespace elysium::game {
std::uint64_t SaveSlotStateCollection::idOf(const SaveSlotState& v) noexcept { return static_cast<std::uint64_t>(v.slotId); }
bool SaveSlotStateCollection::store(SaveSlotState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SaveSlotState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool SaveSlotStateCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SaveSlotState& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const SaveSlotState* SaveSlotStateCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SaveSlotState& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
