#include "fortress/UtilityWorkflow.hpp"

namespace elysium::fortress {

WorkflowPlan runUtilityCycle(const UtilityCycleInput& input,
                             std::uint64_t tick,
                             std::uint32_t producer) {
    WorkflowPlan plan{};
    std::uint32_t sequence = 0;
    auto allocation = allocatePower(input.powerNodes, input.dt);
    if (allocation.demand > allocation.served + 0.001f) {
        plan.events.push_back(workflowEvent(FortressEventKind::PowerBrownout,
                                            TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                            input.controller, input.site,
                                            "Power demand exceeds served load",
                                            allocation.demand - allocation.served));
        for (const auto node : allocation.shedNodes) {
            plan.commands.push(workflowHeader(tick, producer, sequence++),
                               SetPowerIsolationCommand{node, true});
        }
    }

    for (auto& rule : input.automationRules) {
        if (!evaluateAutomationRule(rule, input.signals)) continue;
        plan.commands.push(workflowHeader(tick, producer, sequence++), SetAutomationRuleCommand{rule});
        if (rule.action.value == "elysium:automation/isolate_power") {
            plan.commands.push(workflowHeader(tick, producer, sequence++),
                               SetPowerIsolationCommand{rule.target, true});
        } else if (rule.action.value == "elysium:automation/restore_power") {
            plan.commands.push(workflowHeader(tick, producer, sequence++),
                               SetPowerIsolationCommand{rule.target, false});
        }
    }

    for (auto& mechanism : input.mechanisms) {
        float signal = 0.0f;
        for (const auto& candidate : input.signals) {
            if (candidate.name == mechanism.action) {
                signal = candidate.boolean ? 1.0f : candidate.numeric;
                break;
            }
        }
        (void)updateMechanism(mechanism, signal, false);
    }
    plan.commands.sortDeterministic();
    return plan;
}

} // namespace elysium::fortress
