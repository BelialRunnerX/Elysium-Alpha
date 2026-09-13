#pragma once

#include "fortress/WorkflowCommon.hpp"
#include "fortress/Systems.hpp"

#include <span>

namespace elysium::fortress {

struct OfficeCandidate {
    StableId citizen{};
    const Skills* skills{};
    const Values* values{};
    const Personality* personality{};
    float reputation{};
    bool eligible{true};
};

struct MandateDemand {
    ContentId mandate;
    ContentId labor;
    PriorityBand priority{PriorityBand::Normal};
    float work{1.0f};
};

StableId chooseOfficeHolder(const OfficeState& office,
                            std::span<const OfficeCandidate> candidates);
WorkflowPlan planMandates(const OfficeState& office,
                          std::span<const MandateDemand> demands,
                          std::uint64_t seed,
                          std::uint64_t tick,
                          std::uint32_t producer = 160);

} // namespace elysium::fortress
