#pragma once

#include "fortress/Persistence.hpp"
#include "fortress/SiteLifecycle.hpp"

#include <vector>

namespace elysium::fortress {

struct RetirementPlan {
    SiteId site{};
    TimeStamp retiredAt{};
    std::vector<StableId> persistentEntities;
    std::vector<StableId> compactToHistorical;
    std::vector<StableId> transientEntitiesToRelease;
    std::vector<ArtifactId> artifacts;
    std::vector<InstitutionId> institutions;
    std::uint64_t targetHistoryGeneration{};
    bool spatialSnapshotRequired{true};
    bool strategicSnapshotRequired{true};
};

struct ReclamationPlan {
    SiteId site{};
    TimeStamp reclaimAt{};
    std::vector<StableId> entitiesToPromote;
    std::vector<ArtifactId> artifactsToRestore;
    bool rebuildRooms{true};
    bool rebuildNavigation{true};
    bool rebuildStockIndexes{true};
    bool rebuildRelationshipIndex{true};
    bool reconcileStrategicElapsedTime{true};
};

RetirementPlan makeRetirementPlan(const SiteSnapshot& snapshot, TimeStamp now,
                                  std::uint64_t nextHistoryGeneration);
ReclamationPlan makeReclamationPlan(const SiteSnapshot& snapshot, TimeStamp now);

} // namespace elysium::fortress
