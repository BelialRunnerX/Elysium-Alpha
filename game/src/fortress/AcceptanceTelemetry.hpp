#pragma once

#include "fortress/WorkflowCommon.hpp"

#include <string>
#include <unordered_map>

namespace elysium::fortress {

struct TelemetryCounter {
    std::uint64_t count{};
    double total{};
    double maximum{};
};

struct FortressTelemetry {
    std::unordered_map<std::string, TelemetryCounter> counters;
    std::size_t pendingJobs{};
    std::size_t activeReservations{};
    std::size_t alerts{};
    std::size_t population{};
    std::size_t strategicCohorts{};
};

void recordTelemetry(FortressTelemetry& telemetry, std::string key, double value);
std::vector<WorkflowDiagnostic> auditSimulationBudgets(const FortressTelemetry& telemetry,
                                                       std::size_t maxPendingJobs,
                                                       std::size_t maxReservations,
                                                       double maxObservedQueueLatencyMs);

} // namespace elysium::fortress
