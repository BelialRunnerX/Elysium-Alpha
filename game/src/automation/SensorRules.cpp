// Intended function: Evaluate finite sensor predicates into deterministic automation signals without arbitrary scripting side effects.
#include "SensorRules.hpp"
namespace elysium::automation {
std::uint64_t SensorRuleStateRegistry::key(const SensorRuleState& r) noexcept { return static_cast<std::uint64_t>(r.ruleId); }
bool SensorRuleStateRegistry::publish(SensorRuleState r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const SensorRuleState& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool SensorRuleStateRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const SensorRuleState& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const SensorRuleState* SensorRuleStateRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const SensorRuleState& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::automation
