#pragma once

#include "fortress/InfiltrationSystems.hpp"
#include "fortress/WorkflowCommon.hpp"

#include <span>

namespace elysium::fortress {

struct SecuritySignal {
    StableId actor{};
    float anomaly{};
    float credentialMismatch{};
    float witnessConcern{};
};

WorkflowPlan planCounterintelligence(InfiltrationState infiltration,
                                     SiteId site,
                                     std::span<const SecuritySignal> signals,
                                     float securityPressure,
                                     float socialAccess,
                                     float hours,
                                     std::uint64_t seed,
                                     std::uint64_t tick,
                                     std::uint32_t producer = 330);

} // namespace elysium::fortress
