// Intended function: deterministic on-demand galaxy candidate descriptors over a sparse uint64 address envelope.
#pragma once

#include "galaxy/UniverseFormationFields.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace elysium {

using GalaxySystemId = std::uint64_t;
using GalaxyRegionId = std::uint64_t;
using PlanetCandidateId = std::uint64_t;

enum class StarClass : std::uint8_t { M, K, G, F, A, WhiteDwarf, Neutron, Exotic };

struct GalaxyShapeProfile {
    // All shape values are derived from the universe seed. They are not balance knobs.
    double radialScaleLy{};      // exponential disk scale, not a hard radius
    double verticalScaleLy{};    // exponential/gaussian-ish thickness scale
    double twistRadiansPerScale{};
    double armScatterRadians{};
    double interArmScatterRadians{};
    double interArmFraction{};
    std::uint32_t armCount{};
};

struct GalaxySystemDescriptor {
    GalaxySystemId systemIndex{};
    GalaxyRegionId regionIndex{};
    double xLy{}, yLy{}, zLy{};
    bool starPresent{false};
    double starFormationSignal{};
    double planetFormationBaseline{};
    StarClass starClass{StarClass::G};
    float imperialPressure{};
    float resourceRichness{};
    float stellarAge01{};
    float metallicity01{};
};

struct PlanetCandidateDescriptor {
    GalaxySystemId systemId{};
    PlanetCandidateId candidateId{};
    std::uint64_t stableSeed{};
    PlanetFormationSample formation{};
};

class GalaxyGenerator {
public:
    // There is intentionally no SystemCount. uint64 is a stable address envelope,
    // not a promise to materialize every possible candidate.
    static constexpr std::uint64_t SystemsPerAddressRegion = 256ULL;
    static constexpr GalaxySystemId MaxSystemId = std::numeric_limits<GalaxySystemId>::max();
    static constexpr GalaxyRegionId MaxRegionId = MaxSystemId >> 8U;

    explicit GalaxyGenerator(std::uint64_t galaxySeed, FormationTuning tuning = {});

    [[nodiscard]] std::uint64_t seed() const noexcept { return seed_; }
    [[nodiscard]] const GalaxyShapeProfile& shape() const noexcept { return shape_; }
    [[nodiscard]] std::uint64_t systemSeed(GalaxySystemId index) const;
    [[nodiscard]] std::uint64_t planetSeed(GalaxySystemId index, PlanetCandidateId orbitCandidate) const;
    [[nodiscard]] GalaxySystemDescriptor describe(GalaxySystemId index) const;
    [[nodiscard]] PlanetFormationSample describePlanet(GalaxySystemId index, PlanetCandidateId orbitCandidate) const;
    [[nodiscard]] PlanetCandidateDescriptor planetCandidate(GalaxySystemId system, PlanetCandidateId candidate) const;

    // Lazy finite window over an effectively unbounded candidate sequence. The caller
    // chooses how much to inspect/materialize; this generator defines no planet-count cap.
    [[nodiscard]] std::vector<PlanetCandidateDescriptor> existingPlanetCandidates(
        GalaxySystemId system,
        PlanetCandidateId firstCandidate,
        std::size_t candidatesToInspect,
        std::size_t maxResults = 0) const;

    [[nodiscard]] std::vector<GalaxySystemId> sampleRegion(GalaxyRegionId region, std::size_t maxCount) const;

    [[nodiscard]] const UniverseFormationFields& formation() const noexcept { return formation_; }
    [[nodiscard]] UniverseFormationFields& formation() noexcept { return formation_; }

    static constexpr GalaxyRegionId regionOf(GalaxySystemId system) noexcept { return system >> 8U; }
    static constexpr GalaxySystemId firstSystemOf(GalaxyRegionId region) noexcept {
        const GalaxyRegionId clamped = region > MaxRegionId ? MaxRegionId : region;
        return clamped << 8U;
    }

private:
    [[nodiscard]] UniversePoint candidatePoint(GalaxySystemId index) const;
    [[nodiscard]] GalaxyShapeProfile deriveShape() const;

    std::uint64_t seed_{};
    UniverseFormationFields formation_;
    GalaxyShapeProfile shape_{};
};

} // namespace elysium
