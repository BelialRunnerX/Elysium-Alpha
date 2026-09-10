#include "world/EnvironmentalProjectPolicy.hpp"
#include "core/Saturating.hpp"
namespace elysium::world {namespace{constexpr std::uint64_t Provider=0x454e56454e47ULL;}
EnvironmentalProjectPlan EnvironmentalProjectPolicy::evaluate(const EnvironmentalProjectRequest&r)const{EnvironmentalProjectPlan o{};o.boundedStrength=safe::nonNegative(r.requestedStrength);if(!r.projectId||!r.ownerId||!r.providerId){o.reasons.add({reason::ReasonCode::InvalidRequest,Provider,r.projectId,0});return o;}if(r.kind==EnvironmentalProjectKind::TerraformingCampaignPlaceholder){o.requiresCampaignSystem=true;o.reasons.add({reason::ReasonCode::MissingCapability,Provider,r.projectId,10});return o;}if(r.targetsWholePlanet||!r.targetVolumeId||!r.providerConfirmedBounded){o.reasons.add({reason::ReasonCode::BudgetExceeded,Provider,r.targetVolumeId,20});return o;}o.accepted=true;return o;}
} // namespace elysium::world
