#include "fortress/Systems.hpp"

#include <algorithm>
#include <cmath>

namespace elysium::fortress {

void applyStanding(StandingState& standing, std::uint64_t system, float favorDelta,
                   float suspicionDelta, float suspicionFloor) {
    standing.favor = boundedPercent(standing.favor + favorDelta);
    float& suspicion = standing.suspicionBySystem[system];
    suspicion = std::clamp(suspicion + suspicionDelta, std::max(0.0f, suspicionFloor), 100.0f);
}

float registerActionPressure(float suspicion, float industrialActivity, float defensesVisible) {
    const float s = boundedPercent(suspicion) / 100.0f;
    const float territorial = s < 0.25f ? 0.0f : (s - 0.25f) / 0.75f;
    const float industry = std::sqrt(std::max(0.0f, industrialActivity)) * 0.12f;
    const float fortification = std::sqrt(std::max(0.0f, defensesVisible)) * 0.08f;
    return std::max(0.0f, territorial * 1.2f + industry + fortification);
}

} // namespace elysium::fortress
