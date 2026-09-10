#include "fortress/ExplorationWorkflow.hpp"

namespace elysium::fortress {

WorkflowPlan resolveScan(const ScanAttempt& attempt,
                         DiscoveryDisposition disposition,
                         float dt,
                         std::uint64_t tick,
                         std::uint32_t producer) {
    WorkflowPlan plan{};
    ScanRecord record{};
    record.scanner = attempt.scanner;
    record.targetType = attempt.targetType;
    record.target = attempt.target;
    record.discovery = attempt.discovery;
    record.completeness = saturate(attempt.previousCompleteness +
                                   scanProgress(attempt.scannerGrade, attempt.rangeFactor,
                                                attempt.targetComplexity, attempt.stability, dt));
    record.discovered = TimeStamp{static_cast<std::int64_t>(tick), 0, 0};
    record.filedEmpire = disposition == DiscoveryDisposition::FileEmpire && record.completeness >= 1.0f;
    record.sharedUnsworn = disposition == DiscoveryDisposition::ShareUnsworn && record.completeness >= 1.0f;
    plan.commands.push(workflowHeader(tick, producer, 0), RecordScanCommand{record});
    if (record.completeness >= 1.0f) {
        plan.events.push_back(workflowEvent(FortressEventKind::ScanCompleted, record.discovered,
                                            attempt.target, attempt.site, "Discovery scan completed"));
        if (record.filedEmpire) {
            plan.commands.push(workflowHeader(tick, producer, 1),
                               AdjustStandingCommand{0, 2.0f, 1.0f});
        } else if (record.sharedUnsworn) {
            plan.commands.push(workflowHeader(tick, producer, 1),
                               AdjustStandingCommand{0, 1.0f, 0.0f});
        }
    }
    plan.commands.sortDeterministic();
    return plan;
}

WorkflowPlan enterDirectOperative(StableId citizen,
                                  SiteId site,
                                  ContentId objective,
                                  AlertLevel alert,
                                  std::uint64_t tick,
                                  std::uint32_t producer) {
    WorkflowPlan plan{};
    DirectOperativeState state{};
    state.citizen = citizen;
    state.playerControlled = true;
    state.commandLinkActive = true;
    state.activeSite = site;
    state.objective = std::move(objective);
    state.tacticalAlert = alert;
    plan.commands.push(workflowHeader(tick, producer, 0), SetOperativeCommand{state});
    plan.events.push_back(workflowEvent(FortressEventKind::OperativeIntervention,
                                        TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                        citizen, site, "Direct Operative control entered persistent site"));
    return plan;
}

} // namespace elysium::fortress
