#pragma once

#include "fortress/WorkflowCommon.hpp"
#include "fortress/Systems.hpp"

#include <span>

namespace elysium::fortress {

struct FarmContext {
    SiteId site{};
    StableId farm{};
    float waterAvailability{1.0f};
    float nutrientAvailability{1.0f};
    float climateSuitability{1.0f};
    float feedAvailability{1.0f};
    float pasturePressure{};
};

WorkflowPlan planAgricultureCycle(const FarmContext& context,
                                  std::span<CropState> crops,
                                  std::span<LivestockState> livestock,
                                  float days,
                                  std::uint64_t seed,
                                  std::uint64_t tick,
                                  std::uint32_t producer = 320);

} // namespace elysium::fortress
