#include "fortress/SecurityWorkflow.hpp"

namespace elysium::fortress {
namespace {
constexpr std::uint64_t kSecurityLabel = 0x5345435552495459ULL;
}

WorkflowPlan planCounterintelligence(InfiltrationState infiltration,
                                     SiteId site,
                                     std::span<const SecuritySignal> signals,
                                     float securityPressure,
                                     float socialAccess,
                                     float hours,
                                     std::uint64_t seed,
                                     std::uint64_t tick,
                                     std::uint32_t producer) {
    WorkflowPlan plan{};
    float counterintelligence = 0.0f;
    for (const auto& signal : signals) {
        if (signal.actor != infiltration.actor) continue;
        counterintelligence = std::max(counterintelligence,
            saturate(0.4f * signal.anomaly + 0.35f * signal.credentialMismatch + 0.25f * signal.witnessConcern));
    }
    advanceInfiltration(infiltration, securityPressure, socialAccess, counterintelligence, hours);
    if (infiltration.exposed || infiltration.suspicion > 0.65f) {
        const auto id = makeDerivedId<JobId>(seed, infiltration.actor.value, kSecurityLabel, tick);
        auto investigation = workflowJob(id, "elysium:job/counterintelligence",
                                         ContentId{"elysium:labor/counterintelligence"}, PriorityBand::Urgent,
                                         infiltration.actor, SpatialAnchor{}, 1.5f);
        plan.commands.push(workflowHeader(tick, producer, 0), CreateJobCommand{investigation});
        plan.events.push_back(workflowEvent(FortressEventKind::ThreatArrived,
                                            TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                            infiltration.actor, site, "Infiltration indicators crossed security threshold",
                                            infiltration.suspicion));
    }
    if (infiltration.sabotageTarget && sabotageChance(infiltration, securityPressure, 0.5f) > 0.5f) {
        plan.diagnostics.push_back(WorkflowDiagnostic{
            "sabotage_risk", "Known infiltration has meaningful sabotage opportunity",
            PriorityBand::Emergency, infiltration.sabotageTarget});
    }
    return plan;
}

} // namespace elysium::fortress
