#include "fortress/GovernanceWorkflow.hpp"

namespace elysium::fortress {
namespace {
constexpr std::uint64_t kMandateLabel = 0x4D414E4441544531ULL;
}

StableId chooseOfficeHolder(const OfficeState& office,
                            std::span<const OfficeCandidate> candidates) {
    StableId best{};
    float bestScore = -1.0f;
    for (const auto& candidate : candidates) {
        if (!candidate.eligible || candidate.skills == nullptr || candidate.values == nullptr ||
            candidate.personality == nullptr) {
            continue;
        }
        const float score = officeSuitability(office, *candidate.skills, *candidate.values,
                                              *candidate.personality);
        if (!best || score > bestScore || (score == bestScore && candidate.citizen.value < best.value)) {
            best = candidate.citizen;
            bestScore = score;
        }
    }
    return best;
}

WorkflowPlan planMandates(const OfficeState& office,
                          std::span<const MandateDemand> demands,
                          std::uint64_t seed,
                          std::uint64_t tick,
                          std::uint32_t producer) {
    WorkflowPlan plan{};
    std::uint32_t sequence = 0;
    for (std::size_t i = 0; i < demands.size(); ++i) {
        const auto& demand = demands[i];
        const auto id = makeDerivedId<JobId>(seed, office.holder.value, kMandateLabel,
                                             tick + static_cast<std::uint64_t>(i));
        auto job = workflowJob(id, "elysium:job/administrative_mandate", demand.labor,
                               demand.priority, office.holder, SpatialAnchor{}, demand.work);
        job.materialFilters.push_back(demand.mandate);
        plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{job});
    }
    if (!office.holder && !demands.empty()) {
        plan.diagnostics.push_back(WorkflowDiagnostic{
            "office_vacant", "Mandates exist but office has no holder", PriorityBand::High, {}});
    }
    plan.commands.sortDeterministic();
    return plan;
}

} // namespace elysium::fortress
