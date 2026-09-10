// Intended function: relative resource projections over independently seeded material fields.
#include "galaxy/ResourceDiversityGraph.hpp"

#include "core/Saturating.hpp"

#include <algorithm>
#include <limits>

namespace elysium {

MaterialResourceProjection ResourceDiversityGraph::sample(
    std::uint64_t materialKey,
    const UniversePoint& systemPoint,
    std::uint64_t planetSeed,
    const SpaceResourceContext& space) const {
    const auto s = fields_->sampleResource(materialKey, systemPoint, planetSeed, space);
    return {
        .materialKey = materialKey,
        .universalField = safe::nonNegative(s.rawUniversalField),
        .planetaryAbundance = safe::nonNegative(s.planetaryAbundance),
        .planetaryRelativeShare = 1.0,
        .spaceSourceDensity = safe::nonNegative(s.spaceSourceDensity),
        .spaceRelativeShare = 1.0,
    };
}

std::vector<MaterialResourceProjection> ResourceDiversityGraph::sampleMany(
    std::span<const std::uint64_t> materialKeys,
    const UniversePoint& systemPoint,
    std::uint64_t planetSeed,
    const SpaceResourceContext& space) const {
    std::vector<MaterialResourceProjection> out;
    out.reserve(materialKeys.size());
    double planetSum = 0.0;
    double spaceSum = 0.0;
    for (const auto key : materialKeys) {
        auto value = sample(key, systemPoint, planetSeed, space);
        planetSum = safe::nonNegative(planetSum + value.planetaryAbundance);
        spaceSum = safe::nonNegative(spaceSum + value.spaceSourceDensity);
        out.push_back(value);
    }
    const double pDen = std::max(planetSum, std::numeric_limits<double>::min());
    const double sDen = std::max(spaceSum, std::numeric_limits<double>::min());
    for (auto& value : out) {
        value.planetaryRelativeShare = safe::finiteClamp(value.planetaryAbundance / pDen, 0.0, 1.0);
        value.spaceRelativeShare = spaceSum > 0.0
            ? safe::finiteClamp(value.spaceSourceDensity / sDen, 0.0, 1.0)
            : 0.0;
    }
    return out;
}

} // namespace elysium
