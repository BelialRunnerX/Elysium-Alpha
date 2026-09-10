#include "fortress/FoodSystems.hpp"

#include <algorithm>

namespace elysium::fortress {

float foodPreference(const FoodRecipeState& food, const Preferences& preferences,
                     const Values& values) {
    float affinity{};
    for (const auto& pref : preferences.entries) {
        if (pref.subject == food.recipe) affinity += pref.affinity;
        for (const auto& ingredient : food.ingredients) if (pref.subject == ingredient) affinity += pref.affinity * 0.5f;
    }
    affinity += values.tradition * (food.culture.value.empty() ? 0.0f : 0.15f);
    affinity += values.craft * food.quality * 0.1f;
    return std::clamp(affinity, -1.0f, 2.0f);
}

void advanceFoodSpoilage(FoodRecipeState& food, float hours, float temperature,
                         float contamination, float preservation) {
    const float h = std::max(0.0f, hours);
    const float tempPenalty = std::max(0.0f, temperature - 0.25f) * 0.006f;
    const float contaminationPenalty = saturate(contamination) * 0.008f;
    const float protection = 1.0f - saturate(preservation) * 0.85f;
    food.spoilage = saturate(food.spoilage + h * (0.0015f + tempPenalty + contaminationPenalty) * protection);
    food.quality = saturate(food.quality - h * food.spoilage * 0.0005f);
}

} // namespace elysium::fortress
