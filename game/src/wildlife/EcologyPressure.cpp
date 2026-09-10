// Intended function: Aggregate remote predator/prey, carrying capacity, depletion, invasive pressure, protection, and recolonization.
#include "EcologyPressure.hpp"
namespace elysium::wildlife {
std::uint64_t EcologyPressureStateStore::idOf(const EcologyPressureState& v) noexcept { return static_cast<std::uint64_t>(v.regionId); }
bool EcologyPressureStateStore::put(EcologyPressureState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const EcologyPressureState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool EcologyPressureStateStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const EcologyPressureState& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const EcologyPressureState* EcologyPressureStateStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const EcologyPressureState& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::wildlife
