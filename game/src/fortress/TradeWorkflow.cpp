#include "fortress/TradeWorkflow.hpp"

namespace elysium::fortress {
namespace {
constexpr std::uint64_t kTradeEventLabel = 0x5452414445484953ULL;
}

WorkflowPlan settleCaravanTrade(const CaravanTransaction& transaction,
                                std::uint64_t seed,
                                std::uint64_t tick,
                                std::uint32_t producer) {
    WorkflowPlan plan{};
    std::uint32_t sequence = 0;
    for (const auto& line : transaction.lines) {
        if (!line.item || !line.buyer || line.quantity <= 0.0f) {
            plan.diagnostics.push_back(WorkflowDiagnostic{
                "invalid_trade_line", "Trade line is missing item, buyer, or quantity",
                PriorityBand::High, transaction.caravan});
            continue;
        }
        plan.commands.push(workflowHeader(tick, producer, sequence++),
                           TransferItemCommand{line.item, line.seller, line.buyer, line.quantity});
    }
    HistoricalEvent history{};
    history.id = makeDerivedId<HistoricalEventId>(seed, transaction.caravan.value,
                                                  kTradeEventLabel, tick);
    history.kind = HistoricalEventKind::Discovery;
    history.time = TimeStamp{static_cast<std::int64_t>(tick), 0, 0};
    history.primary = transaction.caravan;
    history.site = transaction.site;
    history.organization = transaction.owner;
    history.summary = "Caravan trade transferred item ownership and manifest state";
    history.significance = transaction.creditsTransferred > 1000.0f ? 0.45f : 0.15f;
    plan.commands.push(workflowHeader(tick, producer, sequence++), RecordHistoricalEventCommand{history});
    plan.events.push_back(workflowEvent(FortressEventKind::CaravanArrived, history.time,
                                        transaction.caravan, transaction.site,
                                        "Caravan transaction committed without implicit item duplication",
                                        transaction.creditsTransferred));
    plan.commands.sortDeterministic();
    return plan;
}

} // namespace elysium::fortress
