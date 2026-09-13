// Intended function: lazily enumerate seed-defined planet candidates without a designed per-star planet-count ceiling.
#pragma once

#include "galaxy/GalaxyGenerator.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace elysium {

struct MaterializedPlanetCandidate {
    GalaxySystemId systemId{};
    std::uint32_t orbitCandidate{};
    std::uint64_t stablePlanetSeed{};
    double formationSignal{};
    double orbitalDistanceAu{};
    double radiusKm{};
};

class PlanetCandidateService final {
public:
    explicit PlanetCandidateService(const GalaxyGenerator& galaxy) : galaxy_(galaxy) {}

    // candidateCount is a caller workload request, not a universe rule.
    // The service never claims that candidates outside this requested window do not exist.
    [[nodiscard]] std::vector<MaterializedPlanetCandidate> discover(
        GalaxySystemId system,
        std::uint32_t firstOrbitCandidate,
        std::size_t candidateCount) const;

    [[nodiscard]] PlanetFormationSample inspect(
        GalaxySystemId system,
        std::uint32_t orbitCandidate) const;

private:
    const GalaxyGenerator& galaxy_;
};

} // namespace elysium
