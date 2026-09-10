// Intended function: turn lazy planet formation samples into seed-derived physical/environmental planet descriptors.
#pragma once

#include "galaxy/GalaxyGenerator.hpp"
#include "world/PlanetTypes.hpp"

#include <cstdint>

namespace elysium {

struct PlanetPhysicalProfile {
    GalaxySystemId systemId{};
    PlanetCandidateId candidateId{};
    std::uint64_t stableSeed{};
    PlanetClass planetClass{PlanetClass::Temperate};
    double orbitalDistanceAu{};
    double radiusKm{};
    double irradiation{};
    double volatilePotential{};
    double oceanPotential{};
    double toxicityPotential{};
    double radiationPotential{};
    double anomalyPotential{};
};

class PlanetFormationProfiler final {
public:
    explicit PlanetFormationProfiler(const GalaxyGenerator& galaxy) : galaxy_(&galaxy) {}
    [[nodiscard]] PlanetPhysicalProfile describe(GalaxySystemId system, PlanetCandidateId candidate) const;

private:
    const GalaxyGenerator* galaxy_{};
};

} // namespace elysium
