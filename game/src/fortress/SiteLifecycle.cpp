#include "fortress/SiteLifecycle.hpp"

#include <algorithm>

namespace elysium::fortress {

SiteLifecycleState evaluateSiteLifecycle(const SiteState& site, const SiteLifecycle& lifecycle,
                                         float activeThreat, float evacuationPressure) {
    if (lifecycle.state == SiteLifecycleState::Retired) return SiteLifecycleState::Retired;
    if (lifecycle.state == SiteLifecycleState::Reclaiming && site.population > 2.0f && site.stability > 0.35f) return SiteLifecycleState::Active;
    if (site.ruined && site.population < 0.5f) return SiteLifecycleState::Ruined;
    if (site.abandoned || site.population < 0.5f) return SiteLifecycleState::Abandoned;
    if (evacuationPressure > 0.8f) return SiteLifecycleState::Evacuating;
    if (activeThreat > 0.65f) return SiteLifecycleState::Besieged;
    const float health = lifecycle.habitability * 0.25f + lifecycle.supplySecurity * 0.25f + lifecycle.defenseSecurity * 0.20f + lifecycle.civicCohesion * 0.30f;
    if (health < 0.35f || site.stability < 0.25f) return SiteLifecycleState::Distressed;
    if (lifecycle.state == SiteLifecycleState::Founding && site.population >= 3.0f) return SiteLifecycleState::Active;
    return lifecycle.state == SiteLifecycleState::Occupied ? SiteLifecycleState::Occupied : SiteLifecycleState::Active;
}

void applySiteLifecycle(SiteState& site, SiteLifecycle& lifecycle, SiteLifecycleState next,
                        TimeStamp time) {
    lifecycle.state = next;
    lifecycle.changed = time;
    site.abandoned = next == SiteLifecycleState::Abandoned || next == SiteLifecycleState::Ruined;
    site.ruined = next == SiteLifecycleState::Ruined;
    site.activeFortress = next == SiteLifecycleState::Active || next == SiteLifecycleState::Besieged ||
                          next == SiteLifecycleState::Distressed || next == SiteLifecycleState::Evacuating ||
                          next == SiteLifecycleState::Reclaiming;
}

} // namespace elysium::fortress
