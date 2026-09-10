#include "fortress/InfiltrationSystems.hpp"

#include <algorithm>

namespace elysium::fortress {

float credentialAccess(const SecurityCredential& credential, std::uint32_t requiredClearance,
                       TimeStamp now) {
    if (credential.expires.tick != 0 && now.tick > credential.expires.tick) return 0.0f;
    if (credential.clearance >= requiredClearance) return 1.0f;
    if (requiredClearance == 0) return 1.0f;
    return saturate(static_cast<float>(credential.clearance) / static_cast<float>(requiredClearance));
}

void advanceInfiltration(InfiltrationState& infiltration, float securityPressure,
                         float socialAccess, float counterintelligence, float hours) {
    if (infiltration.exposed || infiltration.objectiveComplete) return;
    const float h = std::max(0.0f, hours);
    infiltration.accessGained = saturate(infiltration.accessGained + h * 0.01f * saturate(socialAccess) * infiltration.coverStrength);
    infiltration.suspicion = saturate(infiltration.suspicion + h * 0.008f * saturate(securityPressure) * (1.0f - infiltration.coverStrength) +
                                      h * 0.006f * saturate(counterintelligence) - h * 0.002f * saturate(socialAccess));
    infiltration.coverStrength = saturate(infiltration.coverStrength - h * infiltration.suspicion * 0.0015f);
    infiltration.exposed = infiltration.suspicion > 0.92f || infiltration.coverStrength < 0.12f;
}

float sabotageChance(const InfiltrationState& infiltration, float targetSecurity,
                     float targetComplexity) {
    if (infiltration.exposed) return 0.0f;
    const float access = infiltration.accessGained * infiltration.coverStrength;
    const float resistance = 0.35f + saturate(targetSecurity) * 0.4f + saturate(targetComplexity) * 0.25f;
    return saturate(access * (1.0f - resistance * 0.75f));
}

} // namespace elysium::fortress
