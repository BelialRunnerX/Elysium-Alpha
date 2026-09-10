#include "fortress/Retirement.hpp"

namespace elysium::fortress {

RetirementPlan makeRetirementPlan(const SiteSnapshot& snapshot, TimeStamp now,
                                  std::uint64_t nextHistoryGeneration) {
    RetirementPlan plan{};
    plan.site = snapshot.site.id;
    plan.retiredAt = now;
    plan.targetHistoryGeneration = nextHistoryGeneration;
    for (const auto& header : snapshot.entities) {
        if (header.tombstone) continue;
        if (header.shard == SimulationShard::ActiveFortress || header.shard == SimulationShard::DirectOperative) {
            plan.compactToHistorical.push_back(header.stableId);
        } else {
            plan.persistentEntities.push_back(header.stableId);
        }
    }
    for (const auto& artifact : snapshot.artifacts) plan.artifacts.push_back(artifact.artifact);
    return plan;
}

ReclamationPlan makeReclamationPlan(const SiteSnapshot& snapshot, TimeStamp now) {
    ReclamationPlan plan{};
    plan.site = snapshot.site.id;
    plan.reclaimAt = now;
    for (const auto& person : snapshot.people) {
        if (!person.header.tombstone) plan.entitiesToPromote.push_back(person.header.stableId);
    }
    for (const auto& artifact : snapshot.artifacts) plan.artifactsToRestore.push_back(artifact.artifact);
    return plan;
}

} // namespace elysium::fortress
