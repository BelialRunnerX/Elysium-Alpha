#include "fortress/EmergencyWorkflow.hpp"

#include <algorithm>

namespace elysium::fortress {
namespace {
constexpr std::uint64_t kEmergencyLabel = 0x454D455247454E43ULL;

void addEmergencyJob(WorkflowPlan& plan, std::uint64_t seed, std::uint64_t tick,
                     std::uint32_t producer, std::uint32_t& sequence, StableId origin,
                     SiteId site, SpatialAnchor target, std::string type, std::string labor,
                     PriorityBand priority, std::uint64_t salt, float work) {
    const auto id = makeDerivedId<JobId>(seed, origin.value ^ salt, kEmergencyLabel, tick);
    auto job = workflowJob(id, std::move(type), ContentId{std::move(labor)}, priority,
                           origin, target, work);
    job.interruptible = false;
    plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{job});
    plan.events.push_back(workflowEvent(FortressEventKind::JobCreated,
                                        TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                        origin, site, "Emergency job created"));
}
}

WorkflowPlan planEmergencyResponse(const EmergencyContext& context,
                                   std::uint64_t seed,
                                   std::uint64_t tick,
                                   std::uint32_t producer) {
    WorkflowPlan plan{};
    std::uint32_t sequence = 0;
    const bool atmosphereLoss = context.atmosphere.connectedToSky ||
                                context.atmosphere.boundedFillExhausted ||
                                context.atmosphere.pressure < 0.55f ||
                                context.atmosphere.oxygen < 0.15f;
    if (atmosphereLoss) {
        addEmergencyJob(plan, seed, tick, producer, sequence, context.origin, context.site,
                        context.location, "elysium:job/seal_breach", "elysium:labor/sealing",
                        PriorityBand::Emergency, 1, 2.0f);
        addEmergencyJob(plan, seed, tick, producer, sequence, context.origin, context.site,
                        context.location, "elysium:job/restore_atmosphere", "elysium:labor/atmosphere",
                        PriorityBand::Emergency, 2, 1.5f);
        plan.events.push_back(workflowEvent(FortressEventKind::AtmosphereLost,
                                            TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                            context.origin, context.site, "Unsafe room atmosphere detected"));
    }
    if (context.fire != nullptr && context.fire->intensity > 0.05f) {
        addEmergencyJob(plan, seed, tick, producer, sequence, context.origin, context.site,
                        context.location, "elysium:job/fire_suppression", "elysium:labor/fire_response",
                        PriorityBand::Emergency, 3, std::max(1.0f, context.fire->intensity * 4.0f));
        plan.events.push_back(workflowEvent(FortressEventKind::FireStarted,
                                            TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                            context.fire->id, context.site, "Active fire requires response",
                                            context.fire->intensity));
    }
    if (context.contamination != nullptr && context.contamination->intensity > 0.05f) {
        addEmergencyJob(plan, seed, tick, producer, sequence, context.origin, context.site,
                        context.location, "elysium:job/decontaminate", "elysium:labor/decontamination",
                        PriorityBand::Urgent, 4, 1.0f + context.contamination->intensity);
        plan.events.push_back(workflowEvent(FortressEventKind::ContaminationDetected,
                                            TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                            context.contamination->id, context.site,
                                            "Contamination response required",
                                            context.contamination->intensity));
    }
    if (context.civilianExposure > 0.25f && (atmosphereLoss || context.fire != nullptr || context.contamination != nullptr)) {
        addEmergencyJob(plan, seed, tick, producer, sequence, context.origin, context.site,
                        context.location, "elysium:job/evacuate_district", "elysium:labor/evacuation_response",
                        PriorityBand::Emergency, 5, 1.0f);
    }
    plan.commands.sortDeterministic();
    return plan;
}

} // namespace elysium::fortress
