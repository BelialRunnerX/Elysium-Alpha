// Intended function: Track orbital blueprint placement, material delivery, EVA/robotic work, structural progress, power, and pressurization.
#include "OrbitalConstruction.hpp"
namespace elysium::orbital {
std::uint64_t OrbitalBuildJobStore::idOf(const OrbitalBuildJob& v) noexcept { return static_cast<std::uint64_t>(v.jobId); }
bool OrbitalBuildJobStore::put(OrbitalBuildJob v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const OrbitalBuildJob& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool OrbitalBuildJobStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const OrbitalBuildJob& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const OrbitalBuildJob* OrbitalBuildJobStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const OrbitalBuildJob& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::orbital
