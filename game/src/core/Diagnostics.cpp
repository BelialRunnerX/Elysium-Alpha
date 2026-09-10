// Intended function: imported core implementation for Diagnostics; preserves the agent-authored subsystem contract for later integration/debugging.
#include "core/Diagnostics.hpp"

namespace elysium {

const char* diagnosticPhaseName(DiagnosticPhase phase) {
    switch (phase) {
        case DiagnosticPhase::Snapshot: return "snapshot";
        case DiagnosticPhase::Sense: return "sense";
        case DiagnosticPhase::Plan: return "plan";
        case DiagnosticPhase::Resolve: return "resolve";
        case DiagnosticPhase::Commit: return "commit";
        case DiagnosticPhase::Persist: return "persist";
        case DiagnosticPhase::Present: return "present";
        case DiagnosticPhase::Query: return "query";
    }
    return "unknown";
}

const char* diagnosticAccessName(DiagnosticAccess access) {
    switch (access) {
        case DiagnosticAccess::Read: return "read";
        case DiagnosticAccess::Decision: return "decision";
        case DiagnosticAccess::Change: return "change";
        case DiagnosticAccess::Blocked: return "blocked";
    }
    return "unknown";
}

} // namespace elysium
