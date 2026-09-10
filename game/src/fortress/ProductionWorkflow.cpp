#include "fortress/ProductionWorkflow.hpp"

#include <algorithm>
#include <string>

namespace elysium::fortress {
namespace {

constexpr std::uint64_t kProductionJobLabel = 0x50524F445543544EULL;
constexpr std::uint64_t kHaulJobLabel = 0x4841554C494E5054ULL;

const InventoryCandidate* chooseInput(ContentId id, std::span<const InventoryCandidate> inventory,
                                      float quantity) {
    const InventoryCandidate* best = nullptr;
    for (const auto& candidate : inventory) {
        if (candidate.content != id || candidate.forbidden || candidate.reserved ||
            candidate.availableQuantity < quantity) {
            continue;
        }
        if (best == nullptr || candidate.item.value < best->item.value) {
            best = &candidate;
        }
    }
    return best;
}

} // namespace

WorkflowPlan planProductionChain(const ProductionRequest& request,
                                 std::span<const InventoryCandidate> inventory,
                                 std::span<const StockpileComponent> stockpiles,
                                 std::uint64_t seed,
                                 std::uint64_t tick,
                                 std::uint32_t producer) {
    WorkflowPlan plan{};
    std::uint32_t sequence = 0;
    const auto productionId = makeDerivedId<JobId>(seed, request.origin.value, kProductionJobLabel, tick);
    auto production = workflowJob(productionId, "elysium:job/workshop_process", request.labor,
                                  request.priority, request.origin, request.target, request.work);
    production.materialFilters.reserve(request.inputs.size());

    bool missingInput = false;
    for (std::size_t i = 0; i < request.inputs.size(); ++i) {
        const auto& [content, quantity] = request.inputs[i];
        production.materialFilters.push_back(content);
        const auto* source = chooseInput(content, inventory, quantity);
        if (source == nullptr) {
            missingInput = true;
            plan.diagnostics.push_back(WorkflowDiagnostic{
                "missing_input", "No unreserved input for " + content.value, request.priority, request.origin});
            continue;
        }

        const auto haulId = makeDerivedId<JobId>(seed, source->item.value, kHaulJobLabel,
                                                 tick + static_cast<std::uint64_t>(i));
        auto haul = workflowJob(haulId, "elysium:job/haul_workshop",
                                ContentId{"elysium:labor/haul_workshop"}, request.priority,
                                request.origin, request.target, std::max(0.25f, quantity));
        ReservationClaim claim{};
        claim.kind = ReservationKind::Item;
        claim.resource = source->item;
        claim.owner = request.origin;
        claim.job = haulId;
        claim.quantity = quantity;
        claim.expiresTick = static_cast<std::int64_t>(tick + 600);
        haul.reservations.push_back(claim);

        plan.commands.push(workflowHeader(tick, producer, sequence++), ReserveCommand{claim});
        plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{haul});
        production.dependencies.push_back(haulId);
    }

    if (request.workshop) {
        ReservationClaim workshopClaim{};
        workshopClaim.kind = ReservationKind::Workshop;
        workshopClaim.resource = request.workshop;
        workshopClaim.owner = request.origin;
        workshopClaim.job = productionId;
        workshopClaim.expiresTick = static_cast<std::int64_t>(tick + 1200);
        production.reservations.push_back(workshopClaim);
        plan.commands.push(workflowHeader(tick, producer, sequence++), ReserveCommand{workshopClaim});
    }

    if (missingInput) {
        production.state = JobState::Blocked;
        production.failureReason = "required input unavailable";
    }
    plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{production});

    if (!request.outputs.empty()) {
        bool hasOutputDestination = false;
        for (const auto& stockpile : stockpiles) {
            if (stockpile.used < stockpile.capacity) {
                hasOutputDestination = true;
                break;
            }
        }
        if (!hasOutputDestination) {
            plan.diagnostics.push_back(WorkflowDiagnostic{
                "output_storage_full", "No stockpile has free output capacity", PriorityBand::High,
                request.origin});
        }
    }

    plan.events.push_back(workflowEvent(FortressEventKind::JobCreated, TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                        request.origin, request.site,
                                        missingInput ? "Production chain created blocked" : "Production chain created"));
    plan.commands.sortDeterministic();
    return plan;
}

} // namespace elysium::fortress
