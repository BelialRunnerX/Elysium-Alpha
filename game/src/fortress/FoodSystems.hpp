#pragma once

#include "fortress/Components.hpp"

#include <vector>

namespace elysium::fortress {

struct FoodEffect {
    ContentId effect;
    float magnitude{};
    float durationMinutes{};
};

struct FoodRecipeState {
    ContentId recipe;
    std::vector<ContentId> ingredients;
    ContentId culture;
    float calories{};
    float hydration{};
    float quality{};
    float spoilage{};
    std::vector<FoodEffect> effects;
};

float foodPreference(const FoodRecipeState& food, const Preferences& preferences,
                     const Values& values);
void advanceFoodSpoilage(FoodRecipeState& food, float hours, float temperature,
                         float contamination, float preservation);

} // namespace elysium::fortress
