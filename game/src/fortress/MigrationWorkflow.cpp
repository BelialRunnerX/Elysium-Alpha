#include "fortress/MigrationWorkflow.hpp"

#include <algorithm>

namespace elysium::fortress {

MigrationDecision evaluateArrival(const ArrivalDossier& dossier,
                                  const SettlementAttraction& attraction) {
    const float opportunity = saturate(0.25f * attraction.housing + 0.20f * attraction.food +
                                       0.20f * attraction.employment + 0.15f * attraction.institutions +
                                       0.10f * attraction.kinship + 0.10f * attraction.ideology);
    const float danger = saturate(0.65f * attraction.activeThreat + 0.35f * attraction.exclusionPolicy);
    const float personal = saturate(0.35f * dossier.candidate.attraction +
                                    0.25f * dossier.candidate.kinship +
                                    0.20f * dossier.candidate.opportunity +
                                    0.20f * dossier.candidate.ideologicalFit);
    const float score = saturate(0.35f * attraction.safety + 0.35f * opportunity + 0.30f * personal - 0.70f * danger);
    return MigrationDecision{score >= 0.52f, score,
                             score >= 0.52f ? "settlement conditions support arrival" :
                                              "risk, policy, or opportunity insufficient"};
}

WorkflowPlan commitArrival(const ArrivalDossier& dossier,
                           SiteId destination,
                           bool grantResidency,
                           std::uint64_t tick,
                           std::uint32_t producer) {
    WorkflowPlan plan{};
    std::uint32_t sequence = 0;
    auto membership = SettlementMembership{};
    membership.site = destination;
    membership.citizen = grantResidency && !dossier.candidate.visitor;
    membership.resident = grantResidency;
    plan.commands.push(workflowHeader(tick, producer, sequence++), SpawnCitizenCommand{
        dossier.identity, membership, AgeLifeStage{}, BiologicalState{}});
    plan.commands.push(workflowHeader(tick, producer, sequence++), MigratePersonCommand{
        dossier.identity.stableId, dossier.candidate.origin, destination});
    plan.events.push_back(workflowEvent(FortressEventKind::MigrantArrived,
                                        TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                        dossier.identity.stableId, destination,
                                        dossier.candidate.visitor ? "Visitor arrived with persistent dossier" :
                                                                    "Migrant arrived with persistent dossier"));
    if (!grantResidency) {
        plan.diagnostics.push_back(WorkflowDiagnostic{
            "temporary_legal_status", "Arrival remains a visitor/restricted resident",
            PriorityBand::Low, dossier.identity.stableId});
    }
    plan.commands.sortDeterministic();
    return plan;
}

} // namespace elysium::fortress
