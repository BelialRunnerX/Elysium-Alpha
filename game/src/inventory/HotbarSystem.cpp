// Intended function: Track player hotbar slot bindings, active slot, content/item stable identity, quick-use cooldown, and persistence state.
#include "HotbarSystem.hpp"
namespace elysium::inventory {
std::uint64_t HotbarSlotIndex::keyOf(const HotbarSlot& v) noexcept { return static_cast<std::uint64_t>(v.slotId); }
bool HotbarSlotIndex::upsert(HotbarSlot v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const HotbarSlot& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool HotbarSlotIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const HotbarSlot& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const HotbarSlot* HotbarSlotIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const HotbarSlot& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
