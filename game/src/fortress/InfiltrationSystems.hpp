#pragma once

#include "fortress/Components.hpp"

namespace elysium::fortress {

struct InfiltrationState {
    StableId actor{};
    OrganizationId trueFaction{};
    OrganizationId coverFaction{};
    ContentId coverRole;
    float coverStrength{1.0f};
    float suspicion{};
    float accessGained{};
    StableId sabotageTarget{};
    ContentId objective;
    bool exposed{};
    bool objectiveComplete{};
};

float credentialAccess(const SecurityCredential& credential, std::uint32_t requiredClearance,
                       TimeStamp now);
void advanceInfiltration(InfiltrationState& infiltration, float securityPressure,
                         float socialAccess, float counterintelligence, float hours);
float sabotageChance(const InfiltrationState& infiltration, float targetSecurity,
                     float targetComplexity);

} // namespace elysium::fortress
