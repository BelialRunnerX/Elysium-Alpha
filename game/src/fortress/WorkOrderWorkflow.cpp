#include "fortress/WorkOrderWorkflow.hpp"

namespace elysium::fortress {

WorkOrderBatchResult evaluateWorkOrders(std::span<const WorkOrderComponent> orders,
                                        const FortressStockSnapshot& stock,
                                        std::uint64_t seed,
                                        std::uint64_t tick,
                                        std::uint32_t producer) {
    WorkOrderBatchResult result{};
    std::uint32_t sequence = 0;
    for (const auto& order : orders) {
        ++result.evaluated;
        if (!order.enabled) continue;
        if (!workOrderConditionMet(order, stock)) {
            ++result.blockedOrders;
            result.plan.diagnostics.push_back(WorkflowDiagnostic{
                "work_order_condition", order.diagnostic.empty() ? "Work order conditions are false" : order.diagnostic,
                PriorityBand::Low, order.workshopScope});
            continue;
        }
        auto jobs = evaluateWorkOrder(order, stock, seed, tick);
        for (auto& job : jobs) {
            result.plan.commands.push(workflowHeader(tick, producer, sequence++), std::move(job));
            ++result.emittedJobs;
        }
    }
    result.plan.commands.sortDeterministic();
    return result;
}

} // namespace elysium::fortress
