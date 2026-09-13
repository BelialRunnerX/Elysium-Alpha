#include "fortress/EmpireWorkflow.hpp"

namespace elysium::fortress {
namespace {
constexpr std::uint64_t kRegisterLabel = 0x5245474953544552ULL;
}

void applyEmpireObservation(FileKnowledge& knowledge, EmpireObservation observation) {
    observation.fact.confidence = saturate(observation.fact.confidence *
                                           saturate(observation.sensorQuality) *
                                           (1.0f - saturate(observation.concealment)));
    observeFileFact(knowledge, std::move(observation.fact));
}

WorkflowPlan planRegisterPressure(const StandingState& standing,
                                  std::uint64_t system,
                                  SiteId site,
                                  float industrialActivity,
                                  float visibleDefenses,
                                  std::uint64_t seed,
                                  std::uint64_t tick,
                                  std::uint32_t producer) {
    WorkflowPlan plan{};
    const auto it = standing.suspicionBySystem.find(system);
    const float suspicion = it == standing.suspicionBySystem.end() ? 0.0f : it->second;
    const float pressure = registerActionPressure(suspicion, industrialActivity, visibleDefenses);
    if (pressure < 0.25f) return plan;
    ThreatState threat{};
    threat.id = makeDerivedId<StableId>(seed, system, kRegisterLabel, tick);
    threat.kind = ThreatKind::ImperialRegisterAction;
    threat.target = site;
    threat.strength = 1.0f + 4.0f * pressure;
    threat.escalation = pressure;
    threat.alert = pressure >= 0.75f ? AlertLevel::Black : pressure >= 0.5f ? AlertLevel::Red : AlertLevel::Yellow;
    threat.announced = TimeStamp{static_cast<std::int64_t>(tick), 0, 0};
    threat.arrival = TimeStamp{static_cast<std::int64_t>(tick + 600), 0, 0};
    threat.active = true;
    plan.commands.push(workflowHeader(tick, producer, 0), SpawnThreatCommand{threat});
    plan.events.push_back(workflowEvent(FortressEventKind::RegisterActionAnnounced,
                                        threat.announced, threat.id, site,
                                        "Imperial Register Action announced from territorial pressure", pressure));
    return plan;
}

} // namespace elysium::fortress
