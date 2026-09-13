// Intended function: Generate deterministic stable POI descriptors from planet fields, site grammar, rarity, faction, history, and biome constraints.
#include "PoiGenerator.hpp"
namespace elysium::poi {
std::uint64_t PoiDescriptorStore::idOf(const PoiDescriptor& v) noexcept { return static_cast<std::uint64_t>(v.poiId); }
bool PoiDescriptorStore::put(PoiDescriptor v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const PoiDescriptor& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool PoiDescriptorStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const PoiDescriptor& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const PoiDescriptor* PoiDescriptorStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const PoiDescriptor& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::poi
