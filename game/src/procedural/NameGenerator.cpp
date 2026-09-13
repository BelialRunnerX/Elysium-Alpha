// Intended function: Generate deterministic star, planet, region, site, faction, person, ship, artifact, and creature names from labeled seed streams.
#include "NameGenerator.hpp"
namespace elysium::procedural {
std::uint64_t NameSeedTable::keyOf(const NameSeed& v) noexcept { return static_cast<std::uint64_t>(v.nameId); }
bool NameSeedTable::set(NameSeed v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const NameSeed& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool NameSeedTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const NameSeed& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const NameSeed* NameSeedTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const NameSeed& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
