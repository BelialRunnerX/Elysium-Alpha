// Intended function: query independent universe-scale resource fields without collapsing materials to fixed proportions.
#pragma once

#include "galaxy/UniverseFormationFields.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace elysium {

struct MaterialResourceProjection {
    std::uint64_t materialKey{};
    double universalField{};
    double planetaryAbundance{};
    double planetaryRelativeShare{};
    double spaceSourceDensity{};
    double spaceRelativeShare{};
};

class ResourceDiversityGraph final {
public:
    explicit ResourceDiversityGraph(const UniverseFormationFields& fields) : fields_(&fields) {}

    [[nodiscard]] MaterialResourceProjection sample(
        std::uint64_t materialKey,
        const UniversePoint& systemPoint,
        std::uint64_t planetSeed,
        const SpaceResourceContext& space) const;

    [[nodiscard]] std::vector<MaterialResourceProjection> sampleMany(
        std::span<const std::uint64_t> materialKeys,
        const UniversePoint& systemPoint,
        std::uint64_t planetSeed,
        const SpaceResourceContext& space) const;

private:
    const UniverseFormationFields* fields_{};
};

} // namespace elysium
