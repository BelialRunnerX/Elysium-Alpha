#include "automation/AutomationAuthoring.hpp"
#include "core/Saturating.hpp"
namespace elysium::automation {namespace{constexpr std::uint64_t Provider=0x415554484f52ULL;}
RuleInspection AutomationAuthoring::inspect(const TcaRule&r)const{RuleInspection o{r.ruleId,r.trigger,r.action,r.target,r.comparison,safe::finiteClamp(r.threshold,-safe::PublishedScalarCeiling,safe::PublishedScalarCeiling),true,{}};if(!r.ruleId||!r.trigger||!r.action){o.valid=false;o.reasons.add({reason::ReasonCode::InvalidRequest,Provider,r.ruleId,0});}return o;}
bool AutomationAuthoring::publishPreset(RulePreset p,reason::ReasonStack*rs){reason::ReasonStack local;if(!p.presetId||p.rules.empty())local.add({reason::ReasonCode::InvalidRequest,Provider,p.presetId,0});for(const auto&r:p.rules){auto i=inspect(r);local.append(i.reasons);}if(rs)rs->append(local);if(local.blocked())return false;return presets_.emplace(p.presetId,std::move(p)).second;}
const RulePreset*AutomationAuthoring::find(std::uint64_t id)const{auto it=presets_.find(id);return it==presets_.end()?nullptr:&it->second;}
} // namespace elysium::automation
