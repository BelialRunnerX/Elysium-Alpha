// Intended function: Track infectious disease exposure, incubation, symptoms, immunity, transmission pressure, and quarantine policy.
#include "DiseaseSystem.hpp"
namespace elysium::medical {
std::uint64_t DiseaseStateStore::idOf(const DiseaseState& v) noexcept { return static_cast<std::uint64_t>(v.patientId); }
bool DiseaseStateStore::put(DiseaseState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const DiseaseState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool DiseaseStateStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const DiseaseState& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const DiseaseState* DiseaseStateStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const DiseaseState& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::medical
