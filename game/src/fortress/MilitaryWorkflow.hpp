#pragma once

#include "fortress/WorkflowCommon.hpp"
#include "fortress/Systems.hpp"

#include <span>

namespace elysium::fortress {

struct SquadSupplyState {
    SquadId squad{};
    float uniformCompleteness{1.0f};
    float ammunition{1.0f};
    float healthyFraction{1.0f};
    float attendance{1.0f};
};

WorkflowPlan planMilitaryCycle(const SquadState& squad,
                               const SquadSupplyState& supply,
                               SiteId site,
                               float threatStrength,
                               float fortressDefense,
                               float civilianRisk,
                               std::uint64_t seed,
                               std::uint64_t tick,
                               std::uint32_t producer = 130);

} // namespace elysium::fortress
