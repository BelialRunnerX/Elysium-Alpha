#pragma once

#include "fortress/WorkflowCommon.hpp"
#include "fortress/Systems.hpp"

#include <span>

namespace elysium::fortress {

struct DiseaseContact {
    StableId host{};
    float contactIntensity{};
    float filtration{};
    float protection{};
};

WorkflowPlan planDiseaseResponse(const SyndromeState& syndrome,
                                 SiteId site,
                                 std::span<const DiseaseContact> contacts,
                                 std::uint64_t seed,
                                 std::uint64_t tick,
                                 std::uint32_t producer = 230);

} // namespace elysium::fortress
