// Intended function: Resolve biome boundaries/blends from climate, geology, elevation, water, disturbance, and authored overrides.
#include "BiomeTransitions.hpp"
namespace elysium::world {
std::uint64_t BiomeBlendStore::idOf(const BiomeBlend& v) noexcept { return static_cast<std::uint64_t>(v.sampleId); }
bool BiomeBlendStore::put(BiomeBlend v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const BiomeBlend& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool BiomeBlendStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const BiomeBlend& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const BiomeBlend* BiomeBlendStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const BiomeBlend& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::world
