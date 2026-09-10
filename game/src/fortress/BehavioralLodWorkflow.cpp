#include "fortress/BehavioralLodWorkflow.hpp"

#include "fortress/Systems.hpp"

namespace elysium::fortress {

HistoricalPersonRecord demoteNamedCitizen(const PromotionBundle& bundle,
                                           SimulationShard targetShard) {
    auto record = compactHistoricalPerson(bundle.identity, bundle.membership, bundle.age,
                                          bundle.stress, bundle.skills, bundle.relationships,
                                          bundle.memories, bundle.wounds, bundle.uniqueItems,
                                          bundle.artifacts, bundle.offices);
    record.header.shard = targetShard;
    return record;
}

PromotionBundle promoteHistoricalPerson(const HistoricalPersonRecord& record,
                                        SimulationShard targetShard) {
    PromotionBundle bundle{};
    bundle.identity.stableId = record.header.stableId;
    bundle.identity.name = record.name;
    bundle.identity.shard = targetShard;
    bundle.membership = record.membership;
    bundle.age = record.age;
    bundle.stress = record.stress;
    bundle.skills = record.skills;
    bundle.relationships = record.relationships;
    bundle.memories = record.importantMemories;
    bundle.wounds = record.permanentWounds;
    bundle.uniqueItems = record.uniqueItems;
    bundle.artifacts = record.artifacts;
    bundle.offices = record.offices;
    return bundle;
}

RemoteSiteDelta advanceRemoteSiteEquivalent(const SiteState& site,
                                            float days,
                                            float foodFlow,
                                            float goodsFlow,
                                            float threatPressure) {
    auto next = site;
    advanceSite(next, days, foodFlow, goodsFlow, threatPressure);
    return RemoteSiteDelta{site.id,
                           next.population - site.population,
                           next.foodReserve - site.foodReserve,
                           next.industrialCapacity - site.industrialCapacity,
                           next.prosperity - site.prosperity,
                           next.stability - site.stability};
}

} // namespace elysium::fortress
