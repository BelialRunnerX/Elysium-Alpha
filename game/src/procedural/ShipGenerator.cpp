// Intended function: Generate deterministic ship variants from hull archetype, manufacturer/faction style, modules, wear, cargo role, and history.
#include "ShipGenerator.hpp"
namespace elysium::procedural {
std::uint64_t ShipSeedTable::keyOf(const ShipSeed& v) noexcept { return static_cast<std::uint64_t>(v.shipId); }
bool ShipSeedTable::set(ShipSeed v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ShipSeed& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool ShipSeedTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ShipSeed& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const ShipSeed* ShipSeedTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ShipSeed& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
