// Intended function: Generate stable procedural weapon variants from archetype, material, barrel/core, element, affixes, and rarity budget.
#include "ProceduralWeapons.hpp"
namespace elysium::loot {
std::uint64_t ProceduralWeaponStore::idOf(const ProceduralWeapon& v) noexcept { return static_cast<std::uint64_t>(v.weaponId); }
bool ProceduralWeaponStore::put(ProceduralWeapon v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const ProceduralWeapon& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool ProceduralWeaponStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const ProceduralWeapon& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const ProceduralWeapon* ProceduralWeaponStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const ProceduralWeapon& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::loot
