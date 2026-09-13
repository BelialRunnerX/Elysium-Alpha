// Intended function: bounded-workload discovery over an unbounded-by-design procedural orbit candidate sequence.
#include "galaxy/PlanetCandidateService.hpp"

#include "core/Saturating.hpp"

#include <limits>

namespace elysium {

PlanetFormationSample PlanetCandidateService::inspect(
    GalaxySystemId system,
    std::uint32_t orbitCandidate) const {
    return galaxy_.describePlanet(system, orbitCandidate);
}

std::vector<MaterializedPlanetCandidate> PlanetCandidateService::discover(
    GalaxySystemId system,
    std::uint32_t firstOrbitCandidate,
    std::size_t candidateCount) const {
    std::vector<MaterializedPlanetCandidate> out;
    if (candidateCount == 0) return out;

    // Reserve only what size_t can represent; the request is already a local workload.
    out.reserve(candidateCount);
    std::uint32_t orbit = firstOrbitCandidate;
    for (std::size_t i = 0; i < candidateCount; ++i) {
        const auto sample = inspect(system, orbit);
        if (sample.exists) {
            out.push_back(MaterializedPlanetCandidate{
                .systemId = system,
                .orbitCandidate = orbit,
                .stablePlanetSeed = galaxy_.planetSeed(system, orbit),
                .formationSignal = safe::nonNegative(sample.signal),
                .orbitalDistanceAu = safe::nonNegative(sample.orbitalDistanceAu),
                .radiusKm = safe::nonNegative(sample.radiusKm),
            });
        }
        if (orbit == std::numeric_limits<std::uint32_t>::max()) break;
        orbit = safe::saturatingAdd<std::uint32_t>(orbit, 1U);
    }
    return out;
}

} // namespace elysium
