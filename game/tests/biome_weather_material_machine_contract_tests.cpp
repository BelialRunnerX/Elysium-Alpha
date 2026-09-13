#include "content/MachineCatalogue.hpp"
#include "content/MaterialCatalogue.hpp"
#include "world/BiomeDecisionModel.hpp"
#include "world/WeatherRuntime.hpp"
#include "wildlife/EcologyPromotion.hpp"
#include <cassert>
#include <array>
void biome_weather_material_machine_contract_tests_unrun(){
 elysium::PlanetFieldSample field{};field.temperature01=.5;field.moisture01=.5;
 std::array<elysium::BiomeDecisionDefinition,1>b{{{1,0xFFFFFFFFu,.5,.5,.5,.5,.5,.5,1,1,0,2,3,4}}};
 assert(elysium::BiomeDecisionModel{}.select(elysium::PlanetClass::Temperate,field,.5,b).biomeId==1);
 elysium::wildlife::PopulationTendency t{7,10,20,1,0};auto p=elysium::wildlife::EcologyPromotion{}.promote(9,t,4);assert(p.members.size()==4);
 elysium::content::MachineCatalogue m;assert(m.publish({1,2,1,2,4,4,{{5,3,0}},0}));assert(m.supports(1,5));
}
