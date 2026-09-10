// Intended function: Plan staged surgery with diagnosis, procedure prerequisites, surgeon skill, tools, anesthesia, blood, and recovery.
#include "SurgeryPlanner.hpp"
namespace elysium::medical {
std::uint64_t SurgeryPlanStore::idOf(const SurgeryPlan& v) noexcept { return static_cast<std::uint64_t>(v.planId); }
bool SurgeryPlanStore::put(SurgeryPlan v) { auto id=idOf(v); if(!id)return false; auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const SurgeryPlan& a,std::uint64_t b){return idOf(a)<b;}); if(it!=values_.end()&&idOf(*it)==id){*it=v;return true;} values_.insert(it,v); return true; }
bool SurgeryPlanStore::erase(std::uint64_t id) { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const SurgeryPlan& a,std::uint64_t b){return idOf(a)<b;}); if(it==values_.end()||idOf(*it)!=id)return false; values_.erase(it); return true; }
const SurgeryPlan* SurgeryPlanStore::get(std::uint64_t id) const { auto it=std::lower_bound(values_.begin(),values_.end(),id,[](const SurgeryPlan& a,std::uint64_t b){return idOf(a)<b;}); return it!=values_.end()&&idOf(*it)==id?&*it:nullptr; }
} // namespace elysium::medical
