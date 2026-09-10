#pragma once

#include "fortress/WorkflowCommon.hpp"

#include <span>

namespace elysium::fortress {

struct PipelineMergeResult {
    FortressCommandBuffer commands;
    std::vector<FortressEvent> events;
    std::vector<WorkflowDiagnostic> diagnostics;
    std::size_t inputPlans{};
};

PipelineMergeResult mergeWorkflowPlans(std::span<WorkflowPlan> plans,
                                       std::size_t maxCommands,
                                       std::size_t maxEvents);

} // namespace elysium::fortress
