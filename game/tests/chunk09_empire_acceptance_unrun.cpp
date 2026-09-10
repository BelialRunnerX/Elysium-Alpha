#include "ai/CombatDecisionPipeline.hpp"
#include "combat/CombatRules.hpp"
#include "combat/TerrainDamageAdapter.hpp"
#include "court/CourtOfferScheduler.hpp"
#include "faction/ImperialVisibility.hpp"
#include "faction/RegisterActions.hpp"
#include "faction/StandingLedger.hpp"
#include "gear/FieldGearRuntime.hpp"
#include "gear/GearFamilyRuntime.hpp"
#include "orbital/CustomsInspectionPolicy.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

// AG-QA-EMPIRE-001 acceptance fixture.
// Authored deliberately as an unrun integration target during the code-generation
// phase. Production package defects remain owned by their respective systems.
namespace elysium::qa::empire {

void gear_energy_contract() {
    gear::FieldGearRuntime runtime;
    gear::FieldGearInput depleted{};
    depleted.availableEnergy = 0.0;
    depleted.dt = 1.0;
    depleted.industrialExo = true;
    depleted.scanWork = 5.0;
    const auto result = runtime.evaluate(depleted);
    assert(result.energyConsumed >= 0.0);
    assert(result.toolPowerLost || result.energyConsumed == 0.0);
}

void combat_stage_order_contract() {
    DamageContext context{};
    context.eventId = 1;
    context.attackerStableId = 2;
    context.defenderStableId = 3;
    context.baseAmount = 10.0f;
    const auto result = resolveDamage(context);
    const std::array<DamagePipelineStage,9> expected{{
        DamagePipelineStage::DodgeInvulnerability,
        DamagePipelineStage::SpecialMitigation,
        DamagePipelineStage::ElementRelationship,
        DamagePipelineStage::AttackScaling,
        DamagePipelineStage::Critical,
        DamagePipelineStage::DefenseResilience,
        DamagePipelineStage::Reflection,
        DamagePipelineStage::Lifesteal,
        DamagePipelineStage::FinalEvent
    }};
    assert(result.stageOrder == expected);

    context.reflecting = true;
    context.attack.kind = DamageKind::Reflection;
    const auto reflected = resolveDamage(context);
    assert(reflected.reflectionSuppressed || reflected.reflected == 0.0f);
}

void terrain_damage_is_intent_only_contract() {
    combat::TerrainDamageAdapter adapter;
    combat::TerrainHitContext hit{};
    hit.eventId = 10; hit.attackerId = 11; hit.targetAddress = 12;
    hit.category = TerrainDamageCategory::StructuralBreach;
    hit.landedDamage = 1.0e12;
    const auto request = adapter.translate(hit);
    assert(request.eventId == hit.eventId);
    assert(request.targetAddress == hit.targetAddress);
    assert(request.structuralDamage >= 0.0);
    // The adapter produces a request only; persistence remains the world/change-store owner's job.
}

void standing_and_register_contract() {
    faction::StandingLedger standing;
    assert(standing.apply({1,2,3,4,faction::StandingMeter::Suspicion,80.0}));
    standing.setClaimFloor(2,3,25.0);
    assert(standing.find(2,3)->suspicion >= 25.0);

    faction::RegisterActions actions;
    faction::RegisterPolicy marked{};
    marked.band=faction::RegisterBand::Marked;marked.minimumSuspicion=50;marked.preparationTicks=10;marked.waveGapTicks=1;
    marked.waves={{100,0,false},{101,1,false},{102,2,false}};marked.rewardContent=200;marked.pressureRelief=20;marked.successFloorHint=25;
    assert(actions.publish(marked));
    faction::RegisterAction action{};action.id=9;action.player=2;action.system=3;action.claim=4;action.beacon=5;action.seed=6;
    assert(actions.schedule(action,80,0));
    assert(actions.observe(9,0)->preparationWindow);
}

void court_visibility_customs_contract() {
    court::CourtOfferScheduler court{7};
    court::OfferTableDefinition table{};table.tableId=1;table.visibleOfferCount=1;table.entries={{2,court::CourtOfferKind::Exchange,3,4,0,0,1}};
    assert(court.publish(table));
    assert(court.publish(court::CourtOfferScheduler::canonicalEnvoy(court::EnvoyKind::Sentinel,1)));
    assert(court.consider({8,9,0,0},6000));

    faction::ImperialVisibilityService visibility;
    faction::VisibilityInfrastructure archive{1,9,2,3,faction::VisibilityInfrastructureKind::RegionalArchive,faction::VisibilityInfrastructureState::Active,{0,0,0},0,0,0,99,1,false};
    assert(visibility.publish(archive));

    orbital::CustomsInspectionPolicy customs;
    std::array<orbital::ManifestLine,1> manifest{{{1,1,true,false,false,1.0}}};
    const auto decision=customs.inspect({1,2,3,0,1,false,0,0,4,1.0},manifest);
    assert(decision.cleared);

    orbital::InterdictionRegistry interdiction;
    assert(interdiction.create({1,9,2,1,0,100,1,2.0,.5,.25,orbital::InterdictionState::Active}));
    const auto estimate=interdiction.estimate(9,50);
    assert(estimate.alternateRoutePreserved);
    assert(estimate.routeCostMultiplier >= 1.0);
}

void all_unrun() {
    gear_energy_contract();
    combat_stage_order_contract();
    terrain_damage_is_intent_only_contract();
    standing_and_register_contract();
    court_visibility_customs_contract();
}

} // namespace elysium::qa::empire
