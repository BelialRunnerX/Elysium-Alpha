#include "fortress/ObsessionWorkflow.hpp"

#include <algorithm>

namespace elysium::fortress {
namespace {
constexpr std::uint64_t kObsessionJobLabel = 0x4F4253455353494FULL;
constexpr std::uint64_t kArtifactLabel = 0x4152544946414354ULL;
}

WorkflowPlan advanceObsessionWorkflow(const AethericObsession& obsession,
                                      SiteId site,
                                      std::span<const ObsessionMaterialAvailability> materials,
                                      StableId workshop,
                                      float progressIncrement,
                                      std::uint64_t seed,
                                      std::uint64_t tick,
                                      std::uint32_t producer) {
    WorkflowPlan plan{};
    if (obsession.resolved) return plan;
    std::uint32_t sequence = 0;
    auto staged = obsession;
    staged.reservedItems.clear();

    for (const auto& demanded : obsession.demandedMaterials) {
        const auto it = std::find_if(materials.begin(), materials.end(), [&](const auto& material) {
            return material.material == demanded && material.accessible && material.quantity > 0.0f;
        });
        if (it == materials.end()) {
            plan.diagnostics.push_back(WorkflowDiagnostic{
                "obsession_material_missing", "Aetheric Obsession lacks demanded material " + demanded.value,
                PriorityBand::Urgent, obsession.citizen});
            continue;
        }
        staged.reservedItems.push_back(it->item);
    }

    if (staged.reservedItems.size() != obsession.demandedMaterials.size() || !workshop) {
        if (!workshop) {
            plan.diagnostics.push_back(WorkflowDiagnostic{
                "obsession_workshop_missing", "Required obsession workshop is unavailable",
                PriorityBand::Urgent, obsession.citizen});
        }
        return plan;
    }

    const auto jobId = makeDerivedId<JobId>(seed, obsession.citizen.value, kObsessionJobLabel, tick);
    auto job = workflowJob(jobId, "elysium:job/aetheric_obsession",
                           ContentId{"elysium:labor/artifact_craft"}, PriorityBand::Urgent,
                           obsession.citizen, SpatialAnchor{}, 5.0f);
    for (const auto item : staged.reservedItems) {
        ReservationClaim claim{};
        claim.kind = ReservationKind::Item;
        claim.resource = item;
        claim.owner = obsession.citizen;
        claim.job = jobId;
        claim.expiresTick = static_cast<std::int64_t>(tick + 7200);
        job.reservations.push_back(claim);
        plan.commands.push(workflowHeader(tick, producer, sequence++), ReserveCommand{claim});
    }
    ReservationClaim station{};
    station.kind = ReservationKind::Workshop;
    station.resource = workshop;
    station.owner = obsession.citizen;
    station.job = jobId;
    station.expiresTick = static_cast<std::int64_t>(tick + 7200);
    job.reservations.push_back(station);
    plan.commands.push(workflowHeader(tick, producer, sequence++), ReserveCommand{station});
    plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{job});

    staged.progress = saturate(obsession.progress + progressIncrement);
    plan.commands.push(workflowHeader(tick, producer, sequence++), BeginObsessionCommand{staged});
    if (staged.progress >= 1.0f) {
        ArtifactState artifact{};
        artifact.artifact = makeDerivedId<ArtifactId>(seed, obsession.citizen.value, kArtifactLabel, tick);
        artifact.item = StableId{artifact.artifact.value};
        artifact.creator = obsession.citizen;
        artifact.name = "Aetheric Masterwork";
        artifact.value = 1000.0f + 500.0f * obsession.urgency;
        plan.commands.push(workflowHeader(tick, producer, sequence++), CreateArtifactCommand{artifact});
        plan.events.push_back(workflowEvent(FortressEventKind::ArtifactCreated,
                                            TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                            obsession.citizen, site, "Aetheric Obsession produced an artifact",
                                            artifact.value));
    } else {
        plan.events.push_back(workflowEvent(FortressEventKind::ObsessionStarted,
                                            TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                            obsession.citizen, site, "Aetheric Obsession has claimed materials and workshop"));
    }
    plan.commands.sortDeterministic();
    return plan;
}

} // namespace elysium::fortress
