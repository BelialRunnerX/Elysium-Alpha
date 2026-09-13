#include "ai/CombatDecisionPipeline.hpp"
#include "combat/CombatRules.hpp"
#include "combat/TerrainDamageAdapter.hpp"
#include "court/CourtOfferScheduler.hpp"
#include "faction/StandingLedger.hpp"
#include "gear/FieldGearRuntime.hpp"
#include "gear/GearFamilyRuntime.hpp"
#include "orbital/CustomsInspectionPolicy.hpp"
#include <array>
#include <cassert>
void chunk09_combat_empire_contract_tests_unrun(){using namespace elysium;
 DamageContext dc{};dc.eventId=1;dc.attackerStableId=2;dc.defenderStableId=3;dc.baseAmount=10;auto d=resolveDamage(dc);assert(d.landed>=0);
 faction::StandingLedger sl;assert(sl.apply({1,2,3,4,faction::StandingMeter::Suspicion,12}));sl.setClaimFloor(2,3,10);assert(sl.find(2,3)->suspicion>=10);
 gear::GearFamilyRuntime gf;gear::GearModuleDefinition md{};md.moduleId=1;md.occupies={gear::EquipmentChannel::Back};assert(gf.publish(md));assert(gf.evaluate({{1,1,true}}).valid);
 orbital::CustomsInspectionPolicy cp;std::array<orbital::ManifestLine,1>m{{{1,2,true,false,false}}};assert(cp.inspect({1,2,3,0,1,false},m).cleared);
}
