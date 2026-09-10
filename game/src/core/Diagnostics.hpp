// Intended function: imported core implementation for Diagnostics; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include <cstdint>
#include <string>

namespace elysium {

// Common, backend-neutral trace vocabulary for optional developer builds.
// Simulation correctness must never depend on whether a sink is installed.
enum class DiagnosticPhase : std::uint8_t {
    Snapshot = 0,
    Sense = 1,
    Plan = 2,
    Resolve = 3,
    Commit = 4,
    Persist = 5,
    Present = 6,
    Query = 7
};

enum class DiagnosticAccess : std::uint8_t {
    Read = 0,
    Decision = 1,
    Change = 2,
    Blocked = 3
};

struct DiagnosticTraceEvent {
    const char* system{};
    DiagnosticPhase phase{DiagnosticPhase::Query};
    DiagnosticAccess access{DiagnosticAccess::Read};
    std::uint64_t stableId{};
    std::string subject;
    std::string detail;
};

class IDiagnosticTraceSink {
public:
    virtual ~IDiagnosticTraceSink() = default;
    virtual void record(DiagnosticTraceEvent event) = 0;
};

const char* diagnosticPhaseName(DiagnosticPhase phase);
const char* diagnosticAccessName(DiagnosticAccess access);

} // namespace elysium
