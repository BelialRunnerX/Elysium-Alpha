// Intended function: Generate unique artifacts from maker, culture, materials, obsession/event context, function, imagery, and provenance seed.
#include "ArtifactGenerator.hpp"
namespace elysium::procedural {
std::uint64_t ArtifactSeedTable::keyOf(const ArtifactSeed& v) noexcept { return static_cast<std::uint64_t>(v.artifactId); }
bool ArtifactSeedTable::set(ArtifactSeed v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ArtifactSeed& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool ArtifactSeedTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ArtifactSeed& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const ArtifactSeed* ArtifactSeedTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const ArtifactSeed& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
