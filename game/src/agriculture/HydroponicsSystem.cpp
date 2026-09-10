// Intended function: Manage hydroponic beds, nutrients, water, light, pressure, temperature, crop growth, and harvest requests.
#include "HydroponicsSystem.hpp"
namespace elysium::agriculture {
std::uint64_t HydroponicBedStore::idOf(const HydroponicBed& v) noexcept { return static_cast<std::uint64_t>(v.bedId); }
bool HydroponicBedStore::put(HydroponicBed v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const HydroponicBed& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool HydroponicBedStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const HydroponicBed& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const HydroponicBed* HydroponicBedStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const HydroponicBed& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::agriculture
