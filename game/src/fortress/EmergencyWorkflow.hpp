#pragma once

#include "fortress/WorkflowCommon.hpp"

#include <span>

namespace elysium::fortress {

struct EmergencyContext {
    SiteId site{};
    StableId origin{};
    SpatialAnchor location{};
    RoomAtmosphere atmosphere{};
    const FireState* fire{};
    const ContaminationState* contamination{};
    float civilianExposure{};
};

WorkflowPlan planEmergencyResponse(const EmergencyContext& context,
                                   std::uint64_t seed,
                                   std::uint64_t tick,
                                   std::uint32_t producer = 120);

} // namespace elysium::fortress
