#pragma once

#include "fortress/WorkflowCommon.hpp"
#include "fortress/Systems.hpp"

#include <span>

namespace elysium::fortress {

struct CivicDemand {
    StableId citizen{};
    NeedKind need{NeedKind::Social};
    float urgency{};
};

struct InstitutionCapacity {
    const InstitutionState* institution{};
    float staffing{};
    float supplyFraction{};
    float roomQuality{};
    float freeCapacity{};
};

WorkflowPlan planCivicServices(SiteId site,
                               std::span<const CivicDemand> demands,
                               std::span<const InstitutionCapacity> institutions,
                               std::uint64_t seed,
                               std::uint64_t tick,
                               std::uint32_t producer = 220);

} // namespace elysium::fortress
