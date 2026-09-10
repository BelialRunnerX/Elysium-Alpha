// Intended function: Generate deterministic named galaxy regions, arm/interarm classification, Imperial pressure, resources, hazards, and route density.
#include "RegionGenerator.hpp"
namespace elysium::procedural {
std::uint64_t RegionSeedIndex::keyOf(const RegionSeed& v) noexcept { return static_cast<std::uint64_t>(v.regionId); }
bool RegionSeedIndex::upsert(RegionSeed v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const RegionSeed& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool RegionSeedIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const RegionSeed& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const RegionSeed* RegionSeedIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const RegionSeed& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
