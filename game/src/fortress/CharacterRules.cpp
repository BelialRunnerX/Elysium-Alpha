#include "fortress/CharacterRules.hpp"

#include <algorithm>
#include <cmath>

namespace elysium::fortress {

std::uint32_t experienceToNextLevel(std::uint32_t level) {
    return 60U + 40U * level;
}

float combineIndependentMitigation(std::span<const float> protections) {
    float remaining = 1.0f;
    for (const float protection : protections) remaining *= 1.0f - saturate(protection);
    return 1.0f - remaining;
}

float elementAdvantage(Element attack, Element defense) {
    if (attack == Element::Inert || defense == Element::Inert || attack == defense) return 1.0f;
    const auto beats = [](Element a, Element b) {
        switch (a) {
            case Element::Void: return b == Element::Kinetic || b == Element::Dimensional;
            case Element::Plasma: return b == Element::Void || b == Element::Kinetic;
            case Element::Neural: return b == Element::Plasma || b == Element::Void;
            case Element::Dimensional: return b == Element::Neural || b == Element::Plasma;
            case Element::Kinetic: return b == Element::Dimensional || b == Element::Neural;
            case Element::Inert: return false;
        }
        return false;
    };
    return beats(attack, defense) ? 1.25f : beats(defense, attack) ? 0.80f : 1.0f;
}

DamageResult resolveCharacterDamage(const DamageContext& context, std::uint64_t seed,
                                    std::uint64_t eventIdentity) {
    DamageResult result{};
    const auto token = deterministicToken(seed, eventIdentity, 0x44414D414745ULL, 0);
    const float roll = static_cast<float>((token >> 40U) & 0xFFFFFFU) / static_cast<float>(0xFFFFFFU);
    const float dodgeChance = 1.0f - 1.0f / (1.0f + std::max(0.0f, context.defenderReflexes) * 0.018f);
    if (roll < dodgeChance * 0.35f) {
        result.dodged = true;
        return result;
    }
    float damage = std::max(0.0f, context.baseDamage);
    if (context.melee) damage *= 1.0f + std::max(0.0f, context.attackerStrength) * 0.01f;
    damage *= elementAdvantage(context.attackElement, context.defenseElement);
    const float critChance = 1.0f - 1.0f / (1.0f + std::max(0.0f, context.attackerAccuracy) * 0.012f);
    const float critRoll = static_cast<float>((token >> 16U) & 0xFFFFFFU) / static_cast<float>(0xFFFFFFU);
    if (context.criticalEligible && critRoll < critChance * 0.25f) {
        result.critical = true;
        damage *= std::max(1.0f, context.criticalMultiplier);
    }
    result.preMitigation = damage;
    const float resilience = 1.0f - 1.0f / (1.0f + std::max(0.0f, context.defenderResilience) * 0.02f);
    const float armor = std::max(0.0f, context.defenderFortitude) / (std::max(0.0f, context.defenderFortitude) + 100.0f);
    const std::array protections{resilience, armor, saturate(context.independentMitigation)};
    const float mitigation = combineIndependentMitigation(protections);
    result.finalDamage = damage * (1.0f - mitigation);
    result.reflected = result.finalDamage * std::clamp(context.defenderFortitude * 0.0005f, 0.0f, 0.25f);
    return result;
}

} // namespace elysium::fortress
