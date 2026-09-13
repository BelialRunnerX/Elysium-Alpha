#include "fortress/Persistence.hpp"

#include <algorithm>

namespace elysium::fortress {

HistoricalPersonRecord compactHistoricalPerson(const PersistentIdentity& identity,
                                               const SettlementMembership& membership,
                                               const AgeLifeStage& age,
                                               const StressState& stress,
                                               const Skills& skills,
                                               std::span<const RelationshipEdge> relationships,
                                               std::span<const Memory> memories,
                                               std::span<const WoundState> wounds,
                                               std::span<const StableId> uniqueItems,
                                               std::span<const ArtifactId> artifacts,
                                               std::span<const ContentId> offices) {
    HistoricalPersonRecord record{};
    record.header.stableId = identity.stableId;
    record.header.site = membership.site;
    record.header.shard = SimulationShard::GalaxyHistory;
    record.header.schema = SaveSchema{"historical_person_v1", 1};
    record.name = identity.name;
    record.age = age;
    record.membership = membership;
    record.stress = stress;
    record.skills = skills;
    record.relationships.assign(relationships.begin(), relationships.end());
    for (const auto& memory : memories) {
        if (memory.strength > 0.35f || memory.trauma > 0.2f || static_cast<bool>(memory.event)) {
            record.importantMemories.push_back(memory);
        }
    }
    for (const auto& wound : wounds) {
        if (wound.severity >= InjurySeverity::Severe || wound.healing < 0.95f) record.permanentWounds.push_back(wound);
    }
    record.uniqueItems.assign(uniqueItems.begin(), uniqueItems.end());
    record.artifacts.assign(artifacts.begin(), artifacts.end());
    record.offices.assign(offices.begin(), offices.end());
    return record;
}

} // namespace elysium::fortress
