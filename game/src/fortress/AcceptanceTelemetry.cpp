#include "fortress/AcceptanceTelemetry.hpp"

#include <algorithm>

namespace elysium::fortress {

void recordTelemetry(FortressTelemetry& telemetry, std::string key, double value) {
    auto& counter = telemetry.counters[std::move(key)];
    ++counter.count;
    counter.total += value;
    counter.maximum = std::max(counter.maximum, value);
}

std::vector<WorkflowDiagnostic> auditSimulationBudgets(const FortressTelemetry& telemetry,
                                                       std::size_t maxPendingJobs,
                                                       std::size_t maxReservations,
                                                       double maxObservedQueueLatencyMs) {
    std::vector<WorkflowDiagnostic> diagnostics;
    if (telemetry.pendingJobs > maxPendingJobs) {
        diagnostics.push_back(WorkflowDiagnostic{
            "job_queue_budget", "Pending job queue exceeds configured budget",
            PriorityBand::High, {}});
    }
    if (telemetry.activeReservations > maxReservations) {
        diagnostics.push_back(WorkflowDiagnostic{
            "reservation_budget", "Active reservation count exceeds configured budget",
            PriorityBand::High, {}});
    }
    const auto it = telemetry.counters.find("queue_latency_ms");
    if (it != telemetry.counters.end() && it->second.maximum > maxObservedQueueLatencyMs) {
        diagnostics.push_back(WorkflowDiagnostic{
            "queue_latency_budget", "Observed queue latency exceeds acceptance budget",
            PriorityBand::High, {}});
    }
    return diagnostics;
}

} // namespace elysium::fortress
