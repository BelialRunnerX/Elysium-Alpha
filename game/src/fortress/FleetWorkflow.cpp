#include "fortress/FleetWorkflow.hpp"

namespace elysium::fortress {

WorkflowPlan planFleetInterception(const FleetState& attacker,
                                   const FleetState& defender,
                                   SiteId affectedSite,
                                   std::uint64_t seed,
                                   std::uint64_t epoch,
                                   std::uint64_t tick,
                                   std::uint32_t producer) {
    WorkflowPlan plan{};
    const auto result = resolveFleetBattle(attacker, defender, seed, epoch);
    HistoricalEvent event{};
    event.id = makeDerivedId<HistoricalEventId>(seed, attacker.id.value ^ defender.id.value,
                                                0x464C454554424154ULL, epoch);
    event.kind = HistoricalEventKind::Battle;
    event.time = TimeStamp{static_cast<std::int64_t>(tick), 0, 0};
    event.primary = attacker.id;
    event.secondary = defender.id;
    event.site = affectedSite;
    event.organization = result.attackerWon ? attacker.owner : defender.owner;
    event.summary = result.attackerWon ? "Attacking fleet won strategic interception" :
                                         "Defending fleet held strategic interception";
    event.significance = saturate(0.3f + 0.4f * result.collateral +
                                  0.15f * result.attackerLoss + 0.15f * result.defenderLoss);
    plan.commands.push(workflowHeader(tick, producer, 0), RecordHistoricalEventCommand{event});
    plan.events.push_back(workflowEvent(FortressEventKind::HistoricalEventRecorded,
                                        event.time, attacker.id, affectedSite, event.summary,
                                        event.significance));
    return plan;
}

} // namespace elysium::fortress
