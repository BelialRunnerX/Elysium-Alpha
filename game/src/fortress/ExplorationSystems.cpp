#include "fortress/Systems.hpp"

#include <algorithm>

namespace elysium::fortress {

float scanProgress(float scannerGrade, float rangeFactor, float targetComplexity,
                   float interference, float seconds) {
    const float grade = std::max(0.1f, scannerGrade);
    const float range = saturate(rangeFactor);
    const float complexity = std::max(0.1f, targetComplexity);
    const float signal = grade * range * (1.0f - saturate(interference) * 0.8f);
    return std::max(0.0f, seconds) * signal / (complexity * 10.0f);
}

void advanceOperativeObjective(DirectOperativeState& operative, float progress, bool crisis) {
    if (!operative.playerControlled) return;
    if (crisis) operative.tacticalAlert = AlertLevel::Red;
    if (progress >= 1.0f) {
        operative.objective = ContentId{"elysium:objective/completed"};
        if (!crisis) operative.tacticalAlert = AlertLevel::Green;
    }
}

} // namespace elysium::fortress
