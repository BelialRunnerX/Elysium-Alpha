#pragma once

#include "fortress/Components.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace elysium::fortress {

struct SaveSchema {
    std::string name;
    std::uint32_t version{};
};

struct PersistedEntityHeader {
    StableId stableId{};
    SiteId site{};
    SimulationShard shard{SimulationShard::ActiveFortress};
    SaveSchema schema{};
    bool tombstone{};
};

struct HistoricalPersonRecord {
    PersistedEntityHeader header{};
    std::string name;
    AgeLifeStage age{};
    SettlementMembership membership{};
    StressState stress{};
    Skills skills{};
    std::vector<RelationshipEdge> relationships;
    std::vector<Memory> importantMemories;
    std::vector<WoundState> permanentWounds;
    std::vector<StableId> uniqueItems;
    std::vector<ArtifactId> artifacts;
    std::vector<ContentId> offices;
};

struct SiteSnapshot {
    SaveSchema schema{"fortress_site_v1", 1};
    SiteState site{};
    std::vector<PersistedEntityHeader> entities;
    std::vector<HistoricalPersonRecord> people;
    std::vector<HistoricalEvent> history;
    std::vector<ArtifactState> artifacts;
    std::vector<ClaimState> claims;
};

struct SaveManifest {
    SaveSchema schema{"elysium_fortress_manifest_v1", 1};
    std::uint64_t galaxySeed{};
    std::string generatorFingerprint;
    std::vector<SiteId> dirtySites;
    std::vector<std::uint64_t> dirtySystems;
    std::uint64_t historyGeneration{};
};

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
                                               std::span<const ContentId> offices);

} // namespace elysium::fortress
