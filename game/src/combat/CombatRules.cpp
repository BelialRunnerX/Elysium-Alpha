// Intended function: imported combat implementation for CombatRules; preserves the agent-authored subsystem contract for later integration/debugging.
#include "combat/CombatRules.hpp"

#include "core/Determinism.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace elysium {
namespace {

constexpr std::uint64_t kDodgeRollLabel = 0x434F4D4241544447ULL; // COMBATDG
constexpr std::uint64_t kCritRollLabel  = 0x434F4D4241544352ULL; // COMBATCR

float clampShare(float v) {
    return std::clamp(v,0.0f,1.0f);
}

float deterministicRoll(std::uint64_t eventId, std::uint64_t attacker, std::uint64_t defender, std::uint64_t label) {
    std::uint64_t h=mix64(eventId^label);
    h=mix64(h^attacker);
    h=mix64(h^(defender*0x9E3779B97F4A7C15ULL));
    return static_cast<float>((h>>40U)&0xFFFFFFU)/static_cast<float>(0x1000000U);
}

bool isHeatLike(DamageKind kind) {
    return kind==DamageKind::Fire || kind==DamageKind::Explosion;
}

bool isMelee(DamageKind kind) {
    return kind==DamageKind::Melee;
}

} // namespace

float combineShares(float a,float b,float c) {
    a=clampShare(a); b=clampShare(b); c=clampShare(c);
    return 1.0f-(1.0f-a)*(1.0f-b)*(1.0f-c);
}

float tierAdvantage(int tier) {
    static constexpr std::array<float,6> inherited{0.05f,0.10f,0.15f,0.20f,0.30f,0.40f};
    if(tier<=0) return inherited[0];
    if(tier<static_cast<int>(inherited.size())) return inherited[static_cast<std::size_t>(tier)];
    // Continue gaining value while approaching, but never reaching, a full
    // 100% elemental modifier. The exact post-Sovereign curve is tuning; the
    // no-hard-cap behavior is the locked design intent.
    const float extra=static_cast<float>(tier-5);
    return 0.40f + 0.55f*(1.0f-std::exp(-0.16f*extra));
}

bool elementBeats(CombatElement attacker,CombatElement defender) {
    if(attacker==CombatElement::Inert || defender==CombatElement::Inert || attacker==defender) return false;
    switch(attacker) {
        case CombatElement::Void:
            return defender==CombatElement::Kinetic || defender==CombatElement::Dimensional;
        case CombatElement::Plasma:
            return defender==CombatElement::Void || defender==CombatElement::Kinetic;
        case CombatElement::Neural:
            return defender==CombatElement::Plasma || defender==CombatElement::Void;
        case CombatElement::Dimensional:
            return defender==CombatElement::Neural || defender==CombatElement::Plasma;
        case CombatElement::Kinetic:
            return defender==CombatElement::Dimensional || defender==CombatElement::Neural;
        case CombatElement::Inert:
            break;
    }
    return false;
}

DamageResolution resolveDamage(const DamageContext& context) {
    DamageResolution out{};
    out.stageOrder={DamagePipelineStage::DodgeInvulnerability,
                    DamagePipelineStage::SpecialMitigation,
                    DamagePipelineStage::ElementRelationship,
                    DamagePipelineStage::AttackScaling,
                    DamagePipelineStage::Critical,
                    DamagePipelineStage::DefenseResilience,
                    DamagePipelineStage::Reflection,
                    DamagePipelineStage::Lifesteal,
                    DamagePipelineStage::FinalEvent};
    out.event.eventId=context.eventId;
    out.event.attackerStableId=context.attackerStableId;
    out.event.defenderStableId=context.defenderStableId;
    out.event.kind=context.attack.kind;
    out.event.terrainCategory=context.attack.terrainCategory;

    // Whole-handler reflection re-entry guard. Reflected damage is applied by
    // the owner as a direct reflected amount and is never recursively fed back
    // through the pipeline.
    if(context.reflecting || context.attack.kind==DamageKind::Reflection) {
        out.reflectionSuppressed=true;
        out.event.reflection=true;
        return out;
    }

    float amount=std::max(0.0f,context.baseAmount);
    out.incoming=amount;

    // 1. Dodge / invulnerability exceptions.
    out.invulnerable=context.defender.invulnerable && !context.attack.piercesInvulnerability;
    out.dodgeChance=context.attack.piercesInvulnerability?0.0f:
                    std::min(0.60f,combineShares(context.defender.dodgeRune,
                                                 context.defender.dodgeReflex,
                                                 context.defender.dodgePassive));
    if(out.invulnerable || deterministicRoll(context.eventId,context.attackerStableId,
                                             context.defenderStableId,kDodgeRollLabel)<out.dodgeChance) {
        out.dodged=!out.invulnerable;
        out.event.dodged=out.dodged;
        out.event.landed=0.0f;
        return out;
    }

    // 2. Fire/explosion special mitigation.
    if(isHeatLike(context.attack.kind))
        amount*=1.0f-std::min(0.60f,std::max(0.0f,context.defender.heatReduction));
    out.afterSpecialMitigation=amount;

    // 3. Elemental attacker/defender relationship.
    if(elementBeats(context.attack.element,context.defender.element)) {
        amount*=1.0f+tierAdvantage(context.attack.weaponTier)*std::max(0.0f,context.attack.psionicScale);
    } else if(elementBeats(context.defender.element,context.attack.element)) {
        amount*=1.0f-std::min(0.90f,tierAdvantage(context.defender.armorTier)*0.5f*
                                  std::max(0.0f,context.defender.psionicScale));
    }
    out.afterElement=amount;

    // 4. Strength/weapon scaling for melee, then passive attack scaling.
    if(isMelee(context.attack.kind))
        amount+=std::max(0.0f,context.attack.meleeBaseDamage)*std::max(0.0f,context.attack.weaponMultiplier);
    amount*=std::max(0.0f,context.attack.attackScale);
    out.afterAttackScaling=amount;

    // 5. Accuracy critical roll and best passive critical multiplier.
    const float critChance=std::clamp(context.attack.criticalChance,0.0f,0.60f);
    if(critChance>0.0f && deterministicRoll(context.eventId,context.attackerStableId,
                                            context.defenderStableId,kCritRollLabel)<critChance) {
        out.critical=true;
        amount*=std::max(1.0f,context.attack.criticalMultiplier);
    }
    out.afterCritical=amount;

    // 6. Resilience/final defense scaling.
    amount*=1.0f-std::clamp(context.defender.damageReduction,0.0f,0.95f);
    amount*=std::max(0.0f,context.defender.defenceScale);
    out.landed=std::max(0.0f,amount);

    // 7. Reflection uses the incoming figure at this point in the pipeline.
    out.reflected=out.landed*std::clamp(context.defender.reflectShare,0.0f,0.95f);

    // 8. Lifesteal heals only from damage that actually landed.
    out.lifesteal=out.landed*std::clamp(context.attack.lifestealShare,0.0f,0.95f);

    // 9. Final damage event.
    out.event.landed=out.landed;
    out.event.reflected=out.reflected;
    out.event.lifesteal=out.lifesteal;
    out.event.critical=out.critical;
    return out;
}

} // namespace elysium
