#include "construction/PlacementCatalogue.hpp"
#include "core/Saturating.hpp"

namespace elysium::construction {
namespace { constexpr std::uint64_t ProviderPlacement = 0x504c414345ULL; }

bool PlacementCatalogue::publish(PlacementFamily family, reason::ReasonStack* reasons) {
    reason::ReasonStack local;
    if (family.familyId == 0 || family.allowedModesMask == 0) local.add({reason::ReasonCode::InvalidRequest, ProviderPlacement, family.familyId, 0});
    family.footprint.x = safe::nonNegative(family.footprint.x);
    family.footprint.y = safe::nonNegative(family.footprint.y);
    family.footprint.z = safe::nonNegative(family.footprint.z);
    family.minimumSupport01 = safe::finiteClamp(family.minimumSupport01, 0.0, 1.0);
    if (family.rotationSteps == 0) family.rotationSteps = 1;
    if (reasons) reasons->append(local);
    if (local.blocked()) return false;
    return families_.emplace(family.familyId, std::move(family)).second;
}
const PlacementFamily* PlacementCatalogue::find(ContentId id) const { auto it=families_.find(id); return it==families_.end()?nullptr:&it->second; }
PlacementDecision PlacementCatalogue::evaluate(const PlacementProbe& probe) const {
    PlacementDecision out{}; const auto* f=find(probe.familyId);
    if (!f) { out.reasons.add({reason::ReasonCode::UnknownContent,ProviderPlacement,probe.familyId,0}); return out; }
    if ((f->allowedModesMask & placementModeBit(probe.mode)) == 0) out.reasons.add({reason::ReasonCode::InvalidPlacement,ProviderPlacement,probe.familyId,10});
    if (probe.collides) out.reasons.add({reason::ReasonCode::Collision,ProviderPlacement,probe.familyId,20});
    if (!probe.surfaceAvailable && (probe.mode==PlacementMode::Floor || probe.mode==PlacementMode::Wall || probe.mode==PlacementMode::Ceiling))
        out.reasons.add({reason::ReasonCode::UnsupportedSurface,ProviderPlacement,probe.familyId,30});
    if (safe::finiteClamp(probe.support01,0.0,1.0) < f->minimumSupport01) out.reasons.add({reason::ReasonCode::SupportUnsafe,ProviderPlacement,probe.familyId,40});
    out.accepted = !out.reasons.blocked(); return out;
}
} // namespace elysium::construction
