#include "fortress/FortressPipeline.hpp"

#include <algorithm>

namespace elysium::fortress {

PipelineMergeResult mergeWorkflowPlans(std::span<WorkflowPlan> plans,
                                       std::size_t maxCommands,
                                       std::size_t maxEvents) {
    PipelineMergeResult merged{};
    merged.inputPlans = plans.size();
    for (auto& plan : plans) {
        for (auto& command : plan.commands.commands()) {
            if (merged.commands.size() >= maxCommands) {
                merged.diagnostics.push_back(WorkflowDiagnostic{
                    "command_budget", "Workflow command publication budget exhausted",
                    PriorityBand::High, {}});
                break;
            }
            merged.commands.append(std::move(command));
        }
        for (auto& event : plan.events) {
            if (merged.events.size() >= maxEvents) {
                merged.diagnostics.push_back(WorkflowDiagnostic{
                    "event_budget", "Workflow event publication budget exhausted",
                    PriorityBand::High, {}});
                break;
            }
            merged.events.push_back(std::move(event));
        }
        for (auto& diagnostic : plan.diagnostics) {
            merged.diagnostics.push_back(std::move(diagnostic));
        }
    }
    merged.commands.sortDeterministic();
    std::stable_sort(merged.events.begin(), merged.events.end(), [](const FortressEvent& a, const FortressEvent& b) {
        if (a.time.tick != b.time.tick) return a.time.tick < b.time.tick;
        if (a.site != b.site) return a.site.value < b.site.value;
        if (a.primary != b.primary) return a.primary.value < b.primary.value;
        return static_cast<std::uint16_t>(a.kind) < static_cast<std::uint16_t>(b.kind);
    });
    return merged;
}

} // namespace elysium::fortress
