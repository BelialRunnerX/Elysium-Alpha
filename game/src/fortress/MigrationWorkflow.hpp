#pragma once

#include "fortress/WorkflowCommon.hpp"
#include "fortress/Systems.hpp"

#include <span>

namespace elysium::fortress {

struct ArrivalDossier {
    PersistentIdentity identity{};
    MigrationCandidate candidate{};
    Skills skills{};
    std::vector<RelationshipEdge> relationships;
    ContentId legalStatus{"elysium:legal/visitor"};
    std::vector<ItemState> possessions;
};

struct SettlementAttraction {
    float safety{};
    float housing{};
    float food{};
    float employment{};
    float institutions{};
    float kinship{};
    float ideology{};
    float activeThreat{};
    float exclusionPolicy{};
};

MigrationDecision evaluateArrival(const ArrivalDossier& dossier,
                                  const SettlementAttraction& attraction);
WorkflowPlan commitArrival(const ArrivalDossier& dossier,
                           SiteId destination,
                           bool grantResidency,
                           std::uint64_t tick,
                           std::uint32_t producer = 150);

} // namespace elysium::fortress
