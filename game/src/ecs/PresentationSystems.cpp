// Intended function: Publish immutable ECS presentation snapshots for animation, HUD, audio, effects, and renderer interpolation.
#include "PresentationSystems.hpp"
namespace elysium::ecs {
std::uint64_t PresentationSnapshotCollection::idOf(const PresentationSnapshot& v) noexcept { return static_cast<std::uint64_t>(v.snapshotId); }
bool PresentationSnapshotCollection::store(PresentationSnapshot v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const PresentationSnapshot& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool PresentationSnapshotCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const PresentationSnapshot& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const PresentationSnapshot* PresentationSnapshotCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const PresentationSnapshot& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
