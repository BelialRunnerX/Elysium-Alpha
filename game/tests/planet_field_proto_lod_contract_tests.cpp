#include "galaxy/GalaxyGenerator.hpp"
#include "world/PlanetFieldGenerator.hpp"
#include "world/PlanetFormationProfile.hpp"
#include "world/PlanetLodPlanner.hpp"
#include "world/PlanetScale.hpp"
#include "world/ProtoChunkGenerator.hpp"
#include <cassert>

void planet_field_proto_lod_contract_tests_unrun() {
    elysium::GalaxyGenerator galaxy(55);
    auto p = galaxy.planetCandidate(7, 2);
    elysium::PlanetFormationProfiler profiler(galaxy);
    auto phys = profiler.describe(7, 2);
    auto scale = elysium::PlanetScale::fromFormation(p.stableSeed, p.formation);
    elysium::PlanetFieldGenerator fields(55, phys, scale);
    auto a = fields.sample({1,0,0}, 0.0);
    auto b = fields.sample({1,0,0}, 1000.0);
    assert(a.temperature01 >= 0.0 && a.temperature01 <= 1.0);
    assert(a.signedDensity >= b.signedDensity);

    elysium::world::PlanetLodSelector lod;
    auto near = lod.decide({1,100.0,scale.radiusMeters,0.5,1.0,0.0,true,true});
    auto far = lod.decide({2,1.0e8,scale.radiusMeters,0.00001,0.0,0.0,false,false});
    assert(near.sampleStride <= far.sampleStride);
}
