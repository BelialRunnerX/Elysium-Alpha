#include "fortress/Systems.hpp"

#include <algorithm>
#include <cmath>

namespace elysium::fortress {

float craftQuality(float skill, float focus, float toolQuality, float inputQuality,
                   float workshopCondition, float environmentSuitability, float difficulty) {
    const float skillTerm = std::log1p(std::max(0.0f, skill)) / 3.0f;
    const float support = saturate(focus) * 0.20f + saturate(toolQuality) * 0.18f + saturate(inputQuality) * 0.20f +
                          saturate(workshopCondition) * 0.18f + saturate(environmentSuitability) * 0.12f;
    const float challenge = std::max(0.25f, difficulty);
    return std::clamp((skillTerm * 0.45f + support) / challenge, 0.0f, 2.5f);
}

float processThroughput(const MachineState& machine, const RecipeProcess& recipe,
                        float operatorSkill, float operatorFocus) {
    if (!machine.enabled || !machine.powered || machine.faulted || machine.condition <= 0.05f) return 0.0f;
    const float powerFactor = recipe.power <= 0.0f ? 1.0f : saturate(machine.powerDraw / recipe.power);
    const float condition = saturate(machine.condition);
    const float skill = 0.45f + std::log1p(std::max(0.0f, operatorSkill)) * 0.24f;
    const float focus = 0.5f + saturate(operatorFocus) * 0.5f;
    const float heatPenalty = 1.0f - saturate(std::max(0.0f, machine.heat - 0.75f) * 2.5f) * 0.55f;
    const float contaminationPenalty = 1.0f - saturate(machine.contamination) * 0.45f;
    return std::max(0.0f, powerFactor * condition * skill * focus * heatPenalty * contaminationPenalty / std::max(0.05f, recipe.work));
}

void advanceMachineWear(MaintenanceState& maintenance, float work, float contamination,
                        float environmentPenalty) {
    const float load = std::max(0.0f, work);
    const float wear = maintenance.wearRate * load * (1.0f + std::max(0.0f, contamination) * 1.5f + std::max(0.0f, environmentPenalty));
    maintenance.condition = saturate(maintenance.condition - wear);
    maintenance.contaminationPenalty = saturate(contamination);
    maintenance.environmentPenalty = std::max(0.0f, environmentPenalty);
    if (maintenance.condition <= maintenance.warningThreshold) maintenance.serviceRequested = true;
}

bool maintenanceDue(const MaintenanceState& maintenance) {
    return maintenance.serviceRequested || maintenance.condition <= maintenance.warningThreshold;
}

} // namespace elysium::fortress
