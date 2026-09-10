#include "fortress/SiegeWorkflow.hpp"

namespace elysium::fortress {
namespace {
constexpr std::uint64_t kSiegeLabel = 0x5349454745524553ULL;
}

WorkflowPlan planSiegeResponse(const ThreatState& threat,
                               const SiegeDefenseSnapshot& defense,
                               std::span<const SquadState> squads,
                               std::uint64_t seed,
                               std::uint64_t tick,
                               std::uint32_t producer) {
    WorkflowPlan plan{};
    if (!threat.active || threat.defeated) return plan;
    std::uint32_t sequence = 0;
    const float defenseScore = 0.35f * defense.poweredDefense + 0.25f * defense.wallIntegrity +
                               0.20f * defense.shieldCoverage + 0.20f * defense.ammunition;
    const auto alert = chooseSquadAlert(threat.strength, defenseScore, defense.civilianExposure);
    for (const auto& squad : squads) {
        plan.commands.push(workflowHeader(tick, producer, sequence++), SetSquadAlertCommand{squad.id, alert});
    }
    if (defense.wallIntegrity < 0.7f) {
        const auto id = makeDerivedId<JobId>(seed, defense.beacon.value, kSiegeLabel, tick);
        auto repair = workflowJob(id, "elysium:job/emergency_reinforcement",
                                  ContentId{"elysium:labor/repair_structure"}, PriorityBand::Emergency,
                                  defense.beacon, SpatialAnchor{}, 2.0f);
        plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{repair});
    }
    if (defense.ammunition < 0.35f) {
        const auto id = makeDerivedId<JobId>(seed, defense.beacon.value ^ 0x414D4D4FULL,
                                             kSiegeLabel, tick);
        auto ammo = workflowJob(id, "elysium:job/emergency_ammo_haul",
                                ContentId{"elysium:labor/haul_military"}, PriorityBand::Emergency,
                                defense.beacon, SpatialAnchor{}, 1.0f);
        ammo.materialFilters.push_back(ContentId{"elysium:tag/ammunition"});
        plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{ammo});
    }
    if (defense.medicalCapacity < 0.3f) {
        plan.diagnostics.push_back(WorkflowDiagnostic{
            "siege_medical_capacity", "Medical capacity is low for expected siege casualties",
            PriorityBand::Urgent, defense.beacon});
    }
    plan.events.push_back(workflowEvent(FortressEventKind::ThreatArrived,
                                        TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                        threat.id, defense.site, "Siege response staged", threat.strength));
    plan.commands.sortDeterministic();
    return plan;
}

} // namespace elysium::fortress
