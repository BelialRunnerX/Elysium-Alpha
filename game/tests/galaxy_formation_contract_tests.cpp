// Intended function: isolated contract probes for seed-driven sparse galaxy formation. Not cross-integration tested.
#include "core/Saturating.hpp"
#include "galaxy/GalaxyGenerator.hpp"
#include "galaxy/PlanetCandidateService.hpp"
#include "galaxy/ResourceDiversityGraph.hpp"

#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>

using namespace elysium;

int main() {
    constexpr std::uint64_t seed = 0xE1A51A5EEDULL;
    GalaxyGenerator g(seed);
    const auto a = g.describe(42);
    const auto b = g.describe(42);
    assert(a.systemIndex == b.systemIndex);
    assert(a.starPresent == b.starPresent);
    assert(a.starFormationSignal == b.starFormationSignal);
    assert(a.starFormationSignal >= 0.0);
    assert(a.starFormationSignal <= safe::PublishedScalarCeiling);

    FormationTuning doubled{};
    doubled.starFormationMultiplier = 2.0;
    doubled.planetFormationMultiplier = 2.0;
    doubled.resourceAbundanceMultiplier = 2.0;
    UniverseFormationFields f1(seed);
    UniverseFormationFields f2(seed, doubled);
    const UniversePoint p{120.0, -4.0, 820.0};
    const auto s1 = f1.sampleStar(p);
    const auto s2 = f2.sampleStar(p);
    assert(s2.signal == safe::nonNegative(s1.rawSignal * 2.0));

    const auto planet1 = f1.samplePlanet(1234, s1.signal, 7);
    const auto planet2 = f2.samplePlanet(1234, s1.signal, 7);
    assert(planet2.signal == safe::nonNegative(planet1.rawSignal * 2.0));
    assert(planet1.radiusKm >= 0.0);
    assert(planet1.orbitalDistanceAu >= 0.0);

    const std::array<std::uint64_t, 4> materials{11, 22, 33, 44};
    ResourceDiversityGraph r1(f1);
    ResourceDiversityGraph r2(f2);
    const SpaceResourceContext betweenWorlds{.distanceToNearestStarLy=0.0, .distanceToNearestPlanetAu=4.0, .insideStellarSystem=true};
    const SpaceResourceContext nearPlanet{.distanceToNearestStarLy=0.0, .distanceToNearestPlanetAu=0.00001, .insideStellarSystem=true};
    const SpaceResourceContext interstellar{.distanceToNearestStarLy=200.0, .distanceToNearestPlanetAu=1000.0, .insideStellarSystem=false};

    bool foundIndependent = false;
    for (std::size_t i = 1; i < materials.size(); ++i) {
        if (f1.rawMaterialField(materials[i], p) != f1.rawMaterialField(materials[0], p)) foundIndependent = true;
    }
    assert(foundIndependent);

    const auto base = r1.sample(materials[0], p, 9001, betweenWorlds);
    const auto twice = r2.sample(materials[0], p, 9001, betweenWorlds);
    assert(twice.planetaryAbundance == safe::nonNegative(base.planetaryAbundance * 2.0));
    assert(twice.spaceSourceDensity == safe::nonNegative(base.spaceSourceDensity * 2.0));

    const auto near = r1.sample(materials[0], p, 9001, nearPlanet);
    const auto voidSpace = r1.sample(materials[0], p, 9001, interstellar);
    assert(base.spaceSourceDensity >= near.spaceSourceDensity);
    assert(base.spaceSourceDensity >= voidSpace.spaceSourceDensity);
    assert(base.planetaryAbundance == near.planetaryAbundance);
    assert(base.planetaryAbundance == voidSpace.planetaryAbundance);

    const auto profile = r1.sampleMany(materials, p, 9001, betweenWorlds);
    double relativeSum = 0.0;
    for (const auto& row : profile) {
        assert(row.planetaryAbundance >= 0.0);
        assert(row.planetaryRelativeShare >= 0.0 && row.planetaryRelativeShare <= 1.0);
        relativeSum += row.planetaryRelativeShare;
    }
    assert(std::abs(relativeSum - 1.0) < 1e-9);

    UniverseFormationFields hostile(seed, FormationTuning{
        .starFormationMultiplier = std::numeric_limits<double>::infinity(),
        .planetFormationMultiplier = -5.0,
        .resourceAbundanceMultiplier = std::numeric_limits<double>::quiet_NaN(),
    });
    assert(hostile.tuning().starFormationMultiplier <= safe::PublishedScalarCeiling);
    assert(hostile.tuning().planetFormationMultiplier == 0.0);
    assert(hostile.tuning().resourceAbundanceMultiplier == 0.0);

    PlanetCandidateService planets(g);
    const auto discovered = planets.discover(42, std::numeric_limits<std::uint32_t>::max() - 2U, 20);
    for (const auto& row : discovered) {
        assert(row.radiusKm >= 0.0);
        assert(row.orbitalDistanceAu >= 0.0);
    }

    return 0;
}
