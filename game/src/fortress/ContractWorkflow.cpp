#include "fortress/ContractWorkflow.hpp"

namespace elysium::fortress {

WorkflowPlan updateContract(const ContractProgressInput& input,
                            std::uint64_t seed,
                            std::uint64_t tick,
                            std::uint32_t producer) {
    WorkflowPlan plan{};
    auto contract = input.contract;
    if (!contract.accepted || contract.completed || contract.failed) return plan;
    contract.progress = saturate(std::max(contract.progress, input.objectiveProgress));
    contract.failed = input.deadlineExpired || input.complicationFailed;
    contract.completed = !contract.failed && contract.progress >= 1.0f;
    std::uint32_t sequence = 0;
    plan.commands.push(workflowHeader(tick, producer, sequence++), CreateContractCommand{contract});
    if (contract.completed) {
        plan.commands.push(workflowHeader(tick, producer, sequence++),
                           AdjustStandingCommand{0, contract.standingReward, 0.0f});
        HistoricalEvent event{};
        event.id = makeDerivedId<HistoricalEventId>(seed, contract.id.value,
                                                    0x434F4E5452414354ULL, tick);
        event.kind = HistoricalEventKind::Discovery;
        event.time = TimeStamp{static_cast<std::int64_t>(tick), 0, 0};
        event.site = contract.targetSite;
        event.primary = contract.assignee;
        event.organization = contract.issuer;
        event.summary = "Contract completed";
        event.significance = contract.scope == ContractScope::Regional ? 0.65f :
                             contract.scope == ContractScope::System ? 0.4f : 0.2f;
        plan.commands.push(workflowHeader(tick, producer, sequence++), RecordHistoricalEventCommand{event});
        plan.events.push_back(workflowEvent(FortressEventKind::ContractCompleted,
                                            event.time, contract.assignee, contract.targetSite,
                                            "Contract objective completed", contract.credits));
    } else if (contract.failed) {
        plan.diagnostics.push_back(WorkflowDiagnostic{
            "contract_failed", input.deadlineExpired ? "Contract deadline expired" : "Contract complication failed",
            PriorityBand::High, contract.assignee});
    }
    plan.commands.sortDeterministic();
    return plan;
}

} // namespace elysium::fortress
