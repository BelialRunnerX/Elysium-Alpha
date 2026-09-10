// Intended function: Generate deterministic star-system descriptors, star classes, planets, belts, stations, anomalies, routes, and faction pressure.
#include "SystemGenerator.hpp"
namespace elysium::procedural {
std::uint64_t SystemSeedIndex::keyOf(const SystemSeed& v) noexcept { return static_cast<std::uint64_t>(v.systemId); }
bool SystemSeedIndex::upsert(SystemSeed v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SystemSeed& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool SystemSeedIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SystemSeed& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const SystemSeed* SystemSeedIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const SystemSeed& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
