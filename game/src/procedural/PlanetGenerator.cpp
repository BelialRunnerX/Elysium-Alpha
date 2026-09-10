// Intended function: Generate deterministic planet descriptors, class, radius, gravity, atmosphere, resource/climate fields, moons, and generator identity.
#include "PlanetGenerator.hpp"
namespace elysium::procedural {
std::uint64_t PlanetSeedIndex::keyOf(const PlanetSeed& v) noexcept { return static_cast<std::uint64_t>(v.planetId); }
bool PlanetSeedIndex::upsert(PlanetSeed v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const PlanetSeed& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool PlanetSeedIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const PlanetSeed& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const PlanetSeed* PlanetSeedIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const PlanetSeed& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
