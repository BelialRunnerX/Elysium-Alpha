// Intended function: Generate stable asteroid descriptors, composition, mass, rotation, resource veins, hazards, and mining depletion.
#include "AsteroidSystem.hpp"
namespace elysium::orbital {
std::uint64_t AsteroidStateStore::idOf(const AsteroidState& v) noexcept { return static_cast<std::uint64_t>(v.asteroidId); }
bool AsteroidStateStore::put(AsteroidState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const AsteroidState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool AsteroidStateStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const AsteroidState& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const AsteroidState* AsteroidStateStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const AsteroidState& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::orbital
