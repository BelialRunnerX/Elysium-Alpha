#include "gear/GearProgressionTransaction.hpp"
#include "survival/HazardExposure.hpp"
#include "world/EnvironmentalProjectPolicy.hpp"
#include <array>
#include <cassert>
void chunk08_survival_rpg_contract_tests_unrun(){using namespace elysium;
 survival::HazardExposureAggregator h;std::array<survival::HazardSourceSample,1>s{{{1,2,SurvivalHazard::Thermal,10,false}}};std::array<survival::ResistanceSource,2>r{{{3,4,SurvivalHazard::Thermal,.5},{5,6,SurvivalHazard::Thermal,.5}}};auto x=h.evaluate(s,r);assert(x.resistance[0]>.74&&x.effective[0]<2.6);
 world::EnvironmentalProjectPolicy ep;assert(ep.evaluate({1,2,3,4,5,world::EnvironmentalProjectKind::LocalAtmosphere,100,true,false,1}).accepted);assert(!ep.evaluate({1,2,3,4,5,world::EnvironmentalProjectKind::LocalAtmosphere,100,true,true,1}).accepted);
 auto cat=rpg::makeCanonicalRpgCatalog();assert(cat.validateClosedElementGraph());
}
