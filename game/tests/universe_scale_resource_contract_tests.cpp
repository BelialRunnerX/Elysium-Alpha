// Authored contract tests for later execution. Cross integration intentionally deferred.
#include "galaxy/GalaxyGenerator.hpp"
#include "galaxy/ResourceDiversityGraph.hpp"
#include "world/PlanetFormationProfile.hpp"
#include "world/PlanetResourceField.hpp"
#include "world/PlanetScale.hpp"
#include "world/OreVeinSystem.hpp"

#include <cassert>
#include <array>

void universe_scale_resource_contract_tests_unrun() {
    elysium::FormationTuning t{};
    elysium::GalaxyGenerator galaxy(0x123456789ABCDEF0ULL, t);
    const auto shape = galaxy.shape();
    assert(shape.radialScaleLy >= 0.0);
    assert(shape.armCount >= 2);

    const auto p0 = galaxy.planetCandidate(42, 0);
    const auto p1 = galaxy.planetCandidate(42, 1000);
    assert(p0.formation.radiusKm >= 0.0);
    assert(p1.formation.orbitalDistanceAu >= 0.0);

    elysium::UniverseFormationFields fields(1234);
    elysium::ResourceDiversityGraph graph(fields);
    const std::array<std::uint64_t, 3> materials{11,22,33};
    const auto samples = graph.sampleMany(materials, {}, p0.stableSeed, {});
    assert(samples.size() == materials.size());
    for (const auto& s : samples) assert(s.planetaryAbundance >= 0.0);

    const auto scale = elysium::PlanetScale::fromFormation(p0.stableSeed, p0.formation);
    assert(scale.faceColumns >= 1);
    assert(scale.radiusMeters >= 0.0);
}
