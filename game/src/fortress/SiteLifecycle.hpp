#pragma once

#include "fortress/Components.hpp"

namespace elysium::fortress {

enum class SiteLifecycleState : std::uint8_t {
    Founding,
    Active,
    Distressed,
    Besieged,
    Evacuating,
    Abandoned,
    Ruined,
    Occupied,
    Reclaiming,
    Retired
};

struct SiteLifecycle {
    SiteId site{};
    SiteLifecycleState state{SiteLifecycleState::Founding};
    CivilizationId currentOwner{};
    CivilizationId formerOwner{};
    float habitability{1.0f};
    float supplySecurity{1.0f};
    float defenseSecurity{1.0f};
    float civicCohesion{1.0f};
    TimeStamp changed{};
};

SiteLifecycleState evaluateSiteLifecycle(const SiteState& site, const SiteLifecycle& lifecycle,
                                         float activeThreat, float evacuationPressure);
void applySiteLifecycle(SiteState& site, SiteLifecycle& lifecycle, SiteLifecycleState next,
                        TimeStamp time);

} // namespace elysium::fortress
