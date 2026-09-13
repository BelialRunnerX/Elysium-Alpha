#pragma once

#include "fortress/WorkflowCommon.hpp"
#include "fortress/Systems.hpp"

#include <span>

namespace elysium::fortress {

struct SiegeDefenseSnapshot {
    SiteId site{};
    StableId beacon{};
    float poweredDefense{};
    float wallIntegrity{};
    float shieldCoverage{};
    float ammunition{};
    float medicalCapacity{};
    float civilianExposure{};
};

WorkflowPlan planSiegeResponse(const ThreatState& threat,
                               const SiegeDefenseSnapshot& defense,
                               std::span<const SquadState> squads,
                               std::uint64_t seed,
                               std::uint64_t tick,
                               std::uint32_t producer = 250);

} // namespace elysium::fortress
