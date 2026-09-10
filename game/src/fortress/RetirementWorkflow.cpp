#include "fortress/RetirementWorkflow.hpp"

namespace elysium::fortress {
namespace {
constexpr std::uint64_t kRetireLabel = 0x5245544952454D45ULL;
constexpr std::uint64_t kReclaimLabel = 0x5245434C41494D31ULL;
}

WorkflowPlan planFortressRetirement(const SiteSnapshot& snapshot,
                                    TimeStamp now,
                                    std::uint64_t nextHistoryGeneration,
                                    std::uint64_t seed,
                                    std::uint64_t tick,
                                    std::uint32_t producer) {
    WorkflowPlan plan{};
    const auto retire = makeRetirementPlan(snapshot, now, nextHistoryGeneration);
    auto site = snapshot.site;
    site.activeFortress = false;
    plan.commands.push(workflowHeader(tick, producer, 0), UpdateSiteCommand{site});
    HistoricalEvent event{};
    event.id = makeDerivedId<HistoricalEventId>(seed, snapshot.site.id.value, kRetireLabel, tick);
    event.kind = HistoricalEventKind::FortressRetired;
    event.time = now;
    event.site = snapshot.site.id;
    event.summary = "Fortress retired to strategic simulation with persistent identities and touched-world state";
    event.significance = 0.8f;
    plan.commands.push(workflowHeader(tick, producer, 1), RecordHistoricalEventCommand{event});
    plan.diagnostics.push_back(WorkflowDiagnostic{
        "retirement_snapshot", retire.spatialSnapshotRequired ? "Spatial snapshot required before retirement" :
                                                               "No spatial snapshot required",
        PriorityBand::High, {}});
    plan.commands.sortDeterministic();
    return plan;
}

WorkflowPlan planFortressReclamation(const SiteSnapshot& snapshot,
                                     TimeStamp now,
                                     std::uint64_t seed,
                                     std::uint64_t tick,
                                     std::uint32_t producer) {
    WorkflowPlan plan{};
    const auto reclaim = makeReclamationPlan(snapshot, now);
    auto site = snapshot.site;
    site.activeFortress = true;
    site.abandoned = false;
    plan.commands.push(workflowHeader(tick, producer, 0), UpdateSiteCommand{site});
    HistoricalEvent event{};
    event.id = makeDerivedId<HistoricalEventId>(seed, snapshot.site.id.value, kReclaimLabel, tick);
    event.kind = HistoricalEventKind::SiteReclaimed;
    event.time = now;
    event.site = snapshot.site.id;
    event.summary = "Retired fortress reclaimed; active systems must rebuild derived room/nav/stock indexes";
    event.significance = 0.75f;
    plan.commands.push(workflowHeader(tick, producer, 1), RecordHistoricalEventCommand{event});
    if (reclaim.reconcileStrategicElapsedTime) {
        plan.diagnostics.push_back(WorkflowDiagnostic{
            "reconcile_elapsed", "Apply strategic elapsed deltas before active ECS promotion",
            PriorityBand::High, {}});
    }
    plan.commands.sortDeterministic();
    return plan;
}

} // namespace elysium::fortress
