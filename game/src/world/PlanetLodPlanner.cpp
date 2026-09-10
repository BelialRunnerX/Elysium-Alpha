// Intended function: distance/projection/edit-aware LOD with player-edit continuity and saturating outputs.
#include "world/PlanetLodPlanner.hpp"

#include <cmath>

namespace elysium::world {

LodDecision PlanetLodSelector::decide(const LodInput& raw) const noexcept {
    LodInput in = raw;
    in.distanceMeters = elysium::safe::nonNegative(in.distanceMeters);
    in.planetRadiusMeters = elysium::safe::nonNegative(in.planetRadiusMeters);
    in.projectedScreenFraction = elysium::safe::finiteClamp(in.projectedScreenFraction, 0.0, 1.0);
    in.editImportance = elysium::safe::nonNegative(in.editImportance);
    in.hazardImportance = elysium::safe::nonNegative(in.hazardImportance);

    // Reference range is seed/planet-scale agnostic: use radius-relative distance
    // with a 32m minimum so small test planets still behave sanely.
    const double reference = std::max(32.0, std::sqrt(std::max(1.0, in.planetRadiusMeters)) * 4.0);
    const double ratio = std::max(1.0, in.distanceMeters / reference);
    std::uint64_t tier = static_cast<std::uint64_t>(std::floor(std::log2(ratio)));
    // Strong screen contribution and edit/hazard importance pull representation nearer.
    const double importance = in.projectedScreenFraction * 4.0 +
        in.editImportance / (1.0 + in.editImportance) * 3.0 +
        in.hazardImportance / (1.0 + in.hazardImportance) * 2.0 +
        (in.activePhysics ? 4.0 : 0.0);
    const std::uint64_t pull = static_cast<std::uint64_t>(std::floor(std::max(0.0, importance)));
    tier = pull >= tier ? 0ULL : tier - pull;
    tier = std::min<std::uint64_t>(tier, 62ULL); // shift-safety ceiling, not a world-content cap
    const std::uint64_t stride = std::uint64_t{1} << tier;

    const bool macro = tier <= 2 || in.activePhysics;
    const bool micro = tier == 0 && (in.projectedScreenFraction > 0.01 || in.activePhysics);
    const bool editProxy = in.containsPlayerAuthoredChange && !macro;
    const double cost = elysium::safe::nonNegative(
        (macro ? 1.0 : 0.08) * (micro ? 8.0 : 1.0) /
        static_cast<double>(std::max<std::uint64_t>(1ULL, stride)));
    const double priorityD = elysium::safe::nonNegative(
        (in.activePhysics ? 1.0e6 : 0.0) + in.projectedScreenFraction * 1.0e5 +
        in.editImportance * 1.0e3 + in.hazardImportance * 500.0 +
        1.0e4 / (1.0 + in.distanceMeters));
    const auto priority = elysium::safe::saturatingCast<std::uint64_t>(static_cast<std::uint64_t>(priorityD));

    return {
        .request = {
            .addressKey = in.addressKey,
            .distanceBand = ratio,
            .lodTier = tier,
            .priority = priority,
            .estimatedCost = cost,
            .flags = (in.containsPlayerAuthoredChange ? 1ULL : 0ULL) | (in.activePhysics ? 2ULL : 0ULL),
        },
        .sampleStride = stride,
        .requireMacroVoxels = macro,
        .requireMicroDetail = micro,
        .preserveEditProxy = editProxy,
    };
}

std::uint64_t LodRequestStore::keyOf(const LodRequest& value) noexcept { return static_cast<std::uint64_t>(value.addressKey); }

bool LodRequestStore::upsert(LodRequest value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    value.distanceBand = elysium::safe::nonNegative(value.distanceBand);
    value.estimatedCost = elysium::safe::nonNegative(value.estimatedCost);
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const LodRequest& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool LodRequestStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const LodRequest& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const LodRequest* LodRequestStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const LodRequest& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<LodRequest> LodRequestStore::ordered() const { return records_; }

} // namespace elysium::world
