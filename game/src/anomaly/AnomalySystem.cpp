// Intended function: Represent anomalies with discovery state, sensor signatures, effects, research value, hazard, and resolution outcomes.
#include "AnomalySystem.hpp"
namespace elysium::anomaly {
std::uint64_t AnomalyStateStore::idOf(const AnomalyState& v) noexcept { return static_cast<std::uint64_t>(v.anomalyId); }
bool AnomalyStateStore::put(AnomalyState v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const AnomalyState& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool AnomalyStateStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const AnomalyState& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const AnomalyState* AnomalyStateStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const AnomalyState& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::anomaly
