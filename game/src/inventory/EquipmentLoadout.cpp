// Intended function: Track operative worn gear/trinkets/tools/weapons, slot rules, layers, condition, encumbrance, and derived readiness.
#include "EquipmentLoadout.hpp"
namespace elysium::inventory {
std::uint64_t LoadoutStateIndex::keyOf(const LoadoutState& v) noexcept { return static_cast<std::uint64_t>(v.loadoutId); }
bool LoadoutStateIndex::upsert(LoadoutState v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const LoadoutState& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool LoadoutStateIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const LoadoutState& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const LoadoutState* LoadoutStateIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const LoadoutState& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
