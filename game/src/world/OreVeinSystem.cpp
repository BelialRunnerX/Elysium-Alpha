// Intended function: generate sparse deterministic ore-vein descriptors and depletion records without planet-wide dense allocation.
#include "world/OreVeinSystem.hpp"

#include "core/Determinism.hpp"
#include "core/Saturating.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace elysium::world {
namespace {
double unit(std::uint64_t x) noexcept {
    return static_cast<double>((elysium::mix64(x) >> 11U) & ((1ULL << 53U) - 1ULL)) /
           static_cast<double>(1ULL << 53U);
}
} // namespace

GeneratedOreVein OreVeinGenerator::probe(
    const elysium::UniversePoint& systemPoint,
    std::uint64_t planetSeed,
    const OreVeinCandidate& candidate) const {
    const auto resource = local_.sample(candidate.materialKey, systemPoint, planetSeed, candidate.location);
    const auto seed = fields_->subSeed(0x4F52455645494E53ULL, candidate.materialKey, planetSeed, candidate.candidateKey); // OREVEINS
    const double roll = unit(seed ^ 0x11ULL);
    const bool present = roll < resource.veinProbability;
    if (!present) {
        return {.present = false, .vein = {}, .localAbundance = resource.localAbundance, .probability = resource.veinProbability};
    }

    const double radiusMeters = elysium::safe::nonNegative(
        (0.75 + unit(seed ^ 0x22ULL) * 5.25) * (0.45 + resource.localAbundance));
    const double expectedUnitsD = elysium::safe::nonNegative(
        radiusMeters * radiusMeters * radiusMeters * (2.0 + resource.grade * 12.0));
    const auto expectedUnits = elysium::safe::saturatingCast<std::uint64_t>(
        static_cast<std::uint64_t>(std::min(expectedUnitsD,
            static_cast<double>(std::numeric_limits<std::uint64_t>::max()))));
    const auto stable = elysium::mix64(seed ^ 0x5645494E5F4944ULL);
    return {
        .present = true,
        .vein = {
            .stableId = stable == 0 ? 1ULL : stable,
            .oreItemId = candidate.materialKey,
            .grade = elysium::safe::nonNegative(resource.grade),
            .remainingUnits = expectedUnits,
            .radius = radiusMeters,
            .depth = elysium::safe::finiteClamp(candidate.location.depth01, 0.0, 1.0),
        },
        .localAbundance = resource.localAbundance,
        .probability = resource.veinProbability,
    };
}

std::uint64_t OreVeinStore::keyOf(const OreVein& value) noexcept { return static_cast<std::uint64_t>(value.stableId); }

bool OreVeinStore::upsert(OreVein value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    value.grade = elysium::safe::nonNegative(value.grade);
    value.radius = elysium::safe::nonNegative(value.radius);
    value.depth = elysium::safe::finiteClamp(value.depth, 0.0, 1.0);
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const OreVein& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool OreVeinStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const OreVein& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const OreVein* OreVeinStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const OreVein& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<OreVein> OreVeinStore::ordered() const { return records_; }

} // namespace elysium::world
