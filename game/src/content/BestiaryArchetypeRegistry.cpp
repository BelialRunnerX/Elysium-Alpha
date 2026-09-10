#include "content/BestiaryArchetypeRegistry.hpp"
#include "core/Saturating.hpp"
namespace elysium::content {namespace{constexpr std::uint64_t Provider=0x42455354494152ULL;}
bool BestiaryArchetypeRegistry::publish(EnemyArchetype a,reason::ReasonStack*r){reason::ReasonStack q;if(!a.archetypeId||!a.bodyPlanId||a.verbs.empty())q.add({reason::ReasonCode::InvalidRequest,Provider,a.archetypeId,0});a.healthScale=safe::nonNegative(a.healthScale);a.damageScale=safe::nonNegative(a.damageScale);a.speedScale=safe::nonNegative(a.speedScale);a.transformHealthFraction=safe::finiteClamp(a.transformHealthFraction,0.0,1.0);for(auto&v:a.verbs){v.weight=safe::nonNegative(v.weight);v.minimumRange=safe::nonNegative(v.minimumRange);v.maximumRange=std::max(v.minimumRange,safe::nonNegative(v.maximumRange));}if(r)r->append(q);if(q.blocked())return false;return defs_.emplace(a.archetypeId,std::move(a)).second;}
const EnemyArchetype*BestiaryArchetypeRegistry::find(ContentId id)const{auto it=defs_.find(id);return it==defs_.end()?nullptr:&it->second;}
} // namespace elysium::content
