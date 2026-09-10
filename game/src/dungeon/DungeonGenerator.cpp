// Intended function: Generate deterministic dungeon topology, room graph, locks, hazards, encounter tiers, treasure, and exit guarantees.
#include "DungeonGenerator.hpp"
namespace elysium::dungeon {
std::uint64_t DungeonDescriptorStore::idOf(const DungeonDescriptor& v) noexcept { return static_cast<std::uint64_t>(v.dungeonId); }
bool DungeonDescriptorStore::put(DungeonDescriptor v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const DungeonDescriptor& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool DungeonDescriptorStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const DungeonDescriptor& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const DungeonDescriptor* DungeonDescriptorStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const DungeonDescriptor& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::dungeon
