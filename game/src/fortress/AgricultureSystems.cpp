#include "fortress/Systems.hpp"

#include <algorithm>

namespace elysium::fortress {

void advanceCrop(CropState& crop, float days, float ambientSuitability) {
    if (crop.mature) return;
    const float d = std::max(0.0f, days);
    const float resource = std::min({saturate(crop.water), saturate(crop.nutrients), saturate(crop.light), saturate(crop.temperatureSuitability)});
    const float pressure = saturate(crop.diseasePressure * 0.55f + crop.pestPressure * 0.35f);
    const float growthRate = resource * saturate(ambientSuitability) * (1.0f - pressure) * 0.16f;
    crop.growth = saturate(crop.growth + growthRate * d);
    crop.water = saturate(crop.water - 0.11f * d);
    crop.nutrients = saturate(crop.nutrients - 0.06f * d);
    crop.diseasePressure = saturate(crop.diseasePressure + (1.0f - ambientSuitability) * 0.02f * d);
    crop.mature = crop.growth >= 0.999f;
}

void advanceLivestock(LivestockState& livestock, float days, float feedAvailability,
                      float environmentSuitability) {
    const float d = std::max(0.0f, days);
    livestock.hunger = saturate(livestock.hunger + 0.18f * d - saturate(feedAvailability) * 0.22f * d);
    const float stress = livestock.hunger * 0.6f + (1.0f - saturate(environmentSuitability)) * 0.4f;
    livestock.health = saturate(livestock.health - std::max(0.0f, stress - 0.55f) * 0.12f * d);
    livestock.productProgress = saturate(livestock.productProgress + (1.0f - livestock.hunger) * livestock.health * 0.08f * d);
}

float ecologicalPressure(float hunting, float grazing, float pollution, float introducedSpecies,
                         float habitatLoss, float protection) {
    const float pressure = std::max(0.0f, hunting) * 0.22f + std::max(0.0f, grazing) * 0.16f +
                           std::max(0.0f, pollution) * 0.28f + std::max(0.0f, introducedSpecies) * 0.18f +
                           std::max(0.0f, habitatLoss) * 0.30f - std::max(0.0f, protection) * 0.24f;
    return std::clamp(pressure, -1.0f, 3.0f);
}

} // namespace elysium::fortress
