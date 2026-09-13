#include "fortress/MilitaryWorkflow.hpp"

#include <algorithm>

namespace elysium::fortress {
namespace {
constexpr std::uint64_t kMilitaryLabel = 0x4D494C4954415259ULL;
}

WorkflowPlan planMilitaryCycle(const SquadState& squad,
                               const SquadSupplyState& supply,
                               SiteId site,
                               float threatStrength,
                               float fortressDefense,
                               float civilianRisk,
                               std::uint64_t seed,
                               std::uint64_t tick,
                               std::uint32_t producer) {
    WorkflowPlan plan{};
    std::uint32_t sequence = 0;
    const float readiness = squadReadiness(squad, supply.uniformCompleteness,
                                          supply.healthyFraction, supply.attendance);
    const auto desiredAlert = chooseSquadAlert(threatStrength, fortressDefense, civilianRisk);
    if (desiredAlert != squad.alert) {
        plan.commands.push(workflowHeader(tick, producer, sequence++),
                           SetSquadAlertCommand{squad.id, desiredAlert});
    }

    if (supply.uniformCompleteness < 0.95f || supply.ammunition < 0.5f) {
        const auto id = makeDerivedId<JobId>(seed, squad.id.value, kMilitaryLabel, tick);
        auto job = workflowJob(id, "elysium:job/supply_squad",
                               ContentId{"elysium:labor/haul_military"}, PriorityBand::Urgent,
                               squad.commander, SpatialAnchor{}, 1.0f);
        if (supply.uniformCompleteness < 0.95f) {
            job.materialFilters.push_back(ContentId{"elysium:tag/military_equipment"});
            plan.diagnostics.push_back(WorkflowDiagnostic{
                "uniform_incomplete", "Squad equipment profile is incomplete", PriorityBand::High,
                squad.commander});
        }
        if (supply.ammunition < 0.5f) {
            job.materialFilters.push_back(ContentId{"elysium:tag/ammunition"});
            plan.diagnostics.push_back(WorkflowDiagnostic{
                "ammunition_low", "Squad ammunition is below readiness target", PriorityBand::Urgent,
                squad.commander});
        }
        plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{job});
    }

    if (desiredAlert == AlertLevel::Green && squad.training < 0.8f) {
        const auto id = makeDerivedId<JobId>(seed, squad.id.value ^ 0x545241494EULL, kMilitaryLabel, tick);
        auto drill = workflowJob(id, "elysium:job/squad_drill", ContentId{"elysium:labor/drill"},
                                 PriorityBand::Normal, squad.commander, SpatialAnchor{}, 2.0f);
        plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{drill});
    } else if (desiredAlert >= AlertLevel::Red) {
        const auto id = makeDerivedId<JobId>(seed, squad.id.value ^ 0x4D5553544552ULL, kMilitaryLabel, tick);
        auto muster = workflowJob(id, "elysium:job/muster", ContentId{"elysium:labor/muster"},
                                  PriorityBand::Emergency, squad.commander, SpatialAnchor{}, 1.0f);
        muster.interruptible = false;
        plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{muster});
    }

    if (readiness < 0.5f) {
        plan.events.push_back(workflowEvent(FortressEventKind::JobBlocked, TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                            squad.commander, site, "Squad readiness below operational target", readiness));
        plan.diagnostics.push_back(WorkflowDiagnostic{
            "low_readiness", "Squad readiness below 50%", PriorityBand::High, squad.commander});
    }
    plan.commands.sortDeterministic();
    return plan;
}

} // namespace elysium::fortress
