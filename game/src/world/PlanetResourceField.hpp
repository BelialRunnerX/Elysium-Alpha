// Intended function: project universe material abundance into independent local planetary ore fields.
#pragma once

#include "galaxy/ResourceDiversityGraph.hpp"

#include <cstdint>

namespace elysium {

struct PlanetResourcePoint {
    double x01{};
    double y01{};
    double z01{};
    double depth01{};
};

struct PlanetResourceSample {
    std::uint64_t materialKey{};
    double globalAbundance{};
    double localAbundance{};
    double veinProbability{};
    double grade{};
};

class PlanetResourceField final {
public:
    explicit PlanetResourceField(const UniverseFormationFields& fields) : fields_(&fields) {}

    [[nodiscard]] PlanetResourceSample sample(
        std::uint64_t materialKey,
        const UniversePoint& systemPoint,
        std::uint64_t planetSeed,
        const PlanetResourcePoint& point) const;

private:
    [[nodiscard]] double localNoise(
        std::uint64_t materialKey,
        std::uint64_t planetSeed,
        const PlanetResourcePoint& point) const noexcept;

    const UniverseFormationFields* fields_{};
};

} // namespace elysium
