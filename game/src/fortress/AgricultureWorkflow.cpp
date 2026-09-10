#include "fortress/AgricultureWorkflow.hpp"

namespace elysium::fortress {
namespace {
constexpr std::uint64_t kFarmLabel = 0x4641524D4359434CULL;
}

WorkflowPlan planAgricultureCycle(const FarmContext& context,
                                  std::span<CropState> crops,
                                  std::span<LivestockState> livestock,
                                  float days,
                                  std::uint64_t seed,
                                  std::uint64_t tick,
                                  std::uint32_t producer) {
    WorkflowPlan plan{};
    std::uint32_t sequence = 0;
    for (std::size_t i = 0; i < crops.size(); ++i) {
        auto& crop = crops[i];
        crop.water = saturate(crop.water * context.waterAvailability);
        crop.nutrients = saturate(crop.nutrients * context.nutrientAvailability);
        advanceCrop(crop, days, context.climateSuitability);
        if (crop.mature) {
            const auto id = makeDerivedId<JobId>(seed, crop.id.value, kFarmLabel,
                                                 tick + static_cast<std::uint64_t>(i));
            auto harvest = workflowJob(id, "elysium:job/harvest_crop",
                                       ContentId{"elysium:labor/farming"}, PriorityBand::Normal,
                                       context.farm, SpatialAnchor{}, 0.75f);
            plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{harvest});
        } else if (crop.water < 0.25f || crop.nutrients < 0.25f) {
            plan.diagnostics.push_back(WorkflowDiagnostic{
                "crop_resource_deficit", "Crop bed lacks water or nutrients",
                PriorityBand::High, crop.id});
        }
    }
    for (std::size_t i = 0; i < livestock.size(); ++i) {
        auto& animal = livestock[i];
        advanceLivestock(animal, days, context.feedAvailability,
                         context.climateSuitability * (1.0f - 0.5f * saturate(context.pasturePressure)));
        if (animal.hunger > 0.55f) {
            const auto id = makeDerivedId<JobId>(seed, animal.id.value, kFarmLabel,
                                                 tick + 1000 + static_cast<std::uint64_t>(i));
            auto feed = workflowJob(id, "elysium:job/feed_livestock",
                                    ContentId{"elysium:labor/animal_handling"}, PriorityBand::High,
                                    context.farm, SpatialAnchor{}, 0.5f);
            plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{feed});
        }
    }
    if (!plan.commands.empty()) {
        plan.events.push_back(workflowEvent(FortressEventKind::JobCreated,
                                            TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                            context.farm, context.site, "Agriculture cycle generated farm jobs"));
    }
    plan.commands.sortDeterministic();
    return plan;
}

} // namespace elysium::fortress
