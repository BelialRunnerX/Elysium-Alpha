#pragma once

#include "fortress/Persistence.hpp"
#include "fortress/Systems.hpp"
#include "fortress/WorkflowCommon.hpp"

namespace elysium::fortress {

struct PromotionBundle {
    PersistentIdentity identity{};
    SettlementMembership membership{};
    AgeLifeStage age{};
    StressState stress{};
    Skills skills{};
    std::vector<RelationshipEdge> relationships;
    std::vector<Memory> memories;
    std::vector<WoundState> wounds;
    std::vector<StableId> uniqueItems;
    std::vector<ArtifactId> artifacts;
    std::vector<ContentId> offices;
};

HistoricalPersonRecord demoteNamedCitizen(const PromotionBundle& bundle,
                                           SimulationShard targetShard);
PromotionBundle promoteHistoricalPerson(const HistoricalPersonRecord& record,
                                        SimulationShard targetShard);
RemoteSiteDelta advanceRemoteSiteEquivalent(const SiteState& site,
                                            float days,
                                            float foodFlow,
                                            float goodsFlow,
                                            float threatPressure);

} // namespace elysium::fortress
