#pragma once

#include "fortress/WorkflowCommon.hpp"
#include "fortress/Systems.hpp"

#include <span>

namespace elysium::fortress {

struct WorkOrderBatchResult {
    WorkflowPlan plan{};
    std::size_t evaluated{};
    std::size_t emittedJobs{};
    std::size_t blockedOrders{};
};

WorkOrderBatchResult evaluateWorkOrders(std::span<const WorkOrderComponent> orders,
                                        const FortressStockSnapshot& stock,
                                        std::uint64_t seed,
                                        std::uint64_t tick,
                                        std::uint32_t producer = 400);

} // namespace elysium::fortress
