#pragma once

#include "fortress/WorkflowCommon.hpp"

#include <span>

namespace elysium::fortress {

struct ObsessionMaterialAvailability {
    ContentId material;
    StableId item{};
    float quantity{};
    bool accessible{true};
};

WorkflowPlan advanceObsessionWorkflow(const AethericObsession& obsession,
                                      SiteId site,
                                      std::span<const ObsessionMaterialAvailability> materials,
                                      StableId workshop,
                                      float progressIncrement,
                                      std::uint64_t seed,
                                      std::uint64_t tick,
                                      std::uint32_t producer = 240);

} // namespace elysium::fortress
