#include "world/AtmosphereVolume.hpp"
#include "world/ReactiveFluidChemistry.hpp"
#include "world/PlanetClassProfile.hpp"
#include <cassert>

void bounded_fluid_atmosphere_contract_tests_unrun() {
    elysium::VolumeTopologyResult bounded{1,100,false,false,true};
    elysium::VolumeTopologyResult sky{2,10,true,false,false};
    elysium::VolumeTopologyResult budget{3,8192,false,true,false};
    assert(bounded.sealed());
    assert(!sky.sealed());
    assert(!budget.sealed());

    elysium::ReactiveFluidChemistry chemistry;
    auto r=chemistry.waterLava({},0.5,0.25,99);
    assert(r.consumedA==0.25 && r.consumedB==0.25);
    auto acid=chemistry.acidContact(1.0,{{},4,0.5,0.25});
    assert(acid.corrosionDamage>=0.0);
}
