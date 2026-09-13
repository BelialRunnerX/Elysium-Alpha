#pragma once

#include "fortress/WorkflowCommon.hpp"
#include "fortress/Systems.hpp"

#include <span>

namespace elysium::fortress {

struct ResearcherContribution {
    StableId researcher{};
    float skill{};
    float focus{1.0f};
    float facilityQuality{1.0f};
};

WorkflowPlan planResearchCycle(ResearchProject project,
                               SiteId site,
                               std::span<const ResearcherContribution> researchers,
                               float sampleQuality,
                               float hours,
                               ContentId unlock,
                               std::uint64_t seed,
                               std::uint64_t tick,
                               std::uint32_t producer = 310);

} // namespace elysium::fortress
