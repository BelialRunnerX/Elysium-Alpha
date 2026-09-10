// Intended function: Represent future world snapshot sections for chunks, stable objects, entities, strategic summaries, and content fingerprints.
#include "WorldSnapshotProtocol.hpp"
namespace elysium::network {
std::uint64_t WorldSnapshotSectionCollection::idOf(const WorldSnapshotSection& v) noexcept { return static_cast<std::uint64_t>(v.sectionId); }
bool WorldSnapshotSectionCollection::store(WorldSnapshotSection v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const WorldSnapshotSection& a,std::uint64_t b){return idOf(a)<b;}); if(it!=rows_.end()&&idOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool WorldSnapshotSectionCollection::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const WorldSnapshotSection& a,std::uint64_t b){return idOf(a)<b;}); if(it==rows_.end()||idOf(*it)!=id)return false; rows_.erase(it); return true; }
const WorldSnapshotSection* WorldSnapshotSectionCollection::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const WorldSnapshotSection& a,std::uint64_t b){return idOf(a)<b;}); return it!=rows_.end()&&idOf(*it)==id?&*it:nullptr; }
}
