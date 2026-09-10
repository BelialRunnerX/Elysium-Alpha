#include "fortress/CivicWorkflow.hpp"

namespace elysium::fortress {
namespace {
constexpr std::uint64_t kCivicLabel = 0x4349564943535653ULL;

ContentId institutionForNeed(NeedKind need) {
    switch (need) {
        case NeedKind::Food: return ContentId{"elysium:institution/cantina"};
        case NeedKind::Worship: return ContentId{"elysium:institution/shrine"};
        case NeedKind::Learning: return ContentId{"elysium:institution/archive"};
        case NeedKind::Creativity: return ContentId{"elysium:institution/guildhall"};
        case NeedKind::Purpose: return ContentId{"elysium:institution/forum"};
        case NeedKind::Social: return ContentId{"elysium:institution/cantina"};
        default: return ContentId{"elysium:institution/forum"};
    }
}
}

WorkflowPlan planCivicServices(SiteId site,
                               std::span<const CivicDemand> demands,
                               std::span<const InstitutionCapacity> institutions,
                               std::uint64_t seed,
                               std::uint64_t tick,
                               std::uint32_t producer) {
    WorkflowPlan plan{};
    std::uint32_t sequence = 0;
    for (std::size_t i = 0; i < demands.size(); ++i) {
        const auto& demand = demands[i];
        if (demand.urgency < 0.45f) continue;
        const auto desired = institutionForNeed(demand.need);
        const InstitutionCapacity* best = nullptr;
        float bestQuality = -1.0f;
        for (const auto& capacity : institutions) {
            if (capacity.institution == nullptr || capacity.institution->type != desired || capacity.freeCapacity <= 0.0f) {
                continue;
            }
            const float quality = institutionServiceQuality(*capacity.institution, capacity.staffing,
                                                            capacity.supplyFraction, capacity.roomQuality);
            if (best == nullptr || quality > bestQuality) {
                best = &capacity;
                bestQuality = quality;
            }
        }
        if (best == nullptr) {
            plan.diagnostics.push_back(WorkflowDiagnostic{
                "civic_service_missing", "No available institution can serve an urgent citizen need",
                PriorityBand::Normal, demand.citizen});
            continue;
        }
        const auto id = makeDerivedId<JobId>(seed, demand.citizen.value, kCivicLabel,
                                             tick + static_cast<std::uint64_t>(i));
        auto job = workflowJob(id, "elysium:job/civic_participation",
                               ContentId{"elysium:labor/civic_participation"}, PriorityBand::Low,
                               demand.citizen, SpatialAnchor{}, 0.5f + demand.urgency);
        job.origin = best->institution->owner ? best->institution->owner : demand.citizen;
        plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{job});
    }
    if (!plan.commands.empty()) {
        plan.events.push_back(workflowEvent(FortressEventKind::JobCreated, TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                            {}, site, "Civic service activities scheduled", static_cast<float>(plan.commands.size())));
    }
    plan.commands.sortDeterministic();
    return plan;
}

} // namespace elysium::fortress
