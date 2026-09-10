#pragma once

#include "fortress/WorkflowCommon.hpp"
#include "fortress/Systems.hpp"

#include <span>

namespace elysium::fortress {

struct WitnessStatement {
    StableId witness{};
    bool observed{};
    float reliability{1.0f};
};

WorkflowPlan processObservedCrime(const CrimeRecord& crime,
                                  SiteId site,
                                  std::span<const WitnessStatement> witnesses,
                                  float forensicQuality,
                                  float lawSeverity,
                                  float rehabilitationBias,
                                  std::uint64_t seed,
                                  std::uint64_t tick,
                                  std::uint32_t producer = 140);

} // namespace elysium::fortress
