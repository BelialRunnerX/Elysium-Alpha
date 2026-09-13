#pragma once

#include "fortress/MechanismSystems.hpp"
#include "fortress/WorkflowCommon.hpp"
#include "fortress/Systems.hpp"

#include <span>

namespace elysium::fortress {

struct UtilityCycleInput {
    SiteId site{};
    StableId controller{};
    std::span<PowerNode> powerNodes;
    std::span<AutomationRule> automationRules;
    std::span<const AutomationSignal> signals;
    std::span<MechanismState> mechanisms;
    float dt{};
};

WorkflowPlan runUtilityCycle(const UtilityCycleInput& input,
                             std::uint64_t tick,
                             std::uint32_t producer = 200);

} // namespace elysium::fortress
