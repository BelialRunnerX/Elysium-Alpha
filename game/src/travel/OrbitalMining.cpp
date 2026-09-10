// Intended function: Represent orbital mining targets, extraction progress, yield grade, hazard exposure, and cargo destination.
#include "OrbitalMining.hpp"
namespace elysium::travel {
std::uint64_t OrbitalMiningJobRegistry::key(const OrbitalMiningJob& r) noexcept { return static_cast<std::uint64_t>(r.jobId); }
bool OrbitalMiningJobRegistry::publish(OrbitalMiningJob r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const OrbitalMiningJob& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool OrbitalMiningJobRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const OrbitalMiningJob& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const OrbitalMiningJob* OrbitalMiningJobRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const OrbitalMiningJob& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::travel
