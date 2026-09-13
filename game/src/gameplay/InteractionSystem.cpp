// Intended function: Resolve contextual interactions with voxels, machines, actors, items, vehicles, terminals, doors, and dialogue targets.
#include "InteractionSystem.hpp"
namespace elysium::gameplay {
std::uint64_t InteractionTargetStore::idOf(const InteractionTarget& v) noexcept { return static_cast<std::uint64_t>(v.targetId); }
bool InteractionTargetStore::put(InteractionTarget v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const InteractionTarget& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool InteractionTargetStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const InteractionTarget& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const InteractionTarget* InteractionTargetStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const InteractionTarget& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::gameplay
