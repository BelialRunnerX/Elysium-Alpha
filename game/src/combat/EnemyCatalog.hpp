// Intended function: imported combat implementation for EnemyCatalog; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "combat/CombatRules.hpp"

#include <cstdint>
#include <string_view>

namespace elysium {

enum class EnemyFaction : std::uint8_t { Unsworn = 0, Empire = 1 };

enum class EnemyArchetype : std::uint8_t {
    Scavenger = 0,
    Reaver = 1,
    Whisper = 2,
    Drone = 3,
    Lictor = 4,
    Adept = 5,
    Praetor = 6,
    Choir = 7
};

enum class EnemyRank : std::uint8_t { Standard = 0, Elite = 1, Boss = 2 };

enum class EnemyAbility : std::uint32_t {
    None = 0,
    Cornered = 1u << 0,
    Knitting = 1u << 1,
    Swift = 1u << 2,
    Venomous = 1u << 3,
    Unshaken = 1u << 4,
    Hardened = 1u << 5,
    Deadfall = 1u << 6,
    Sundering = 1u << 7,
    Bulwark = 1u << 8,
    StandardSupport = 1u << 9,
    Echo = 1u << 10,
    Overcharged = 1u << 11,
    Blinding = 1u << 12,
    ElementalHardened = 1u << 13
};

constexpr EnemyAbility operator|(EnemyAbility a,EnemyAbility b) {
    return static_cast<EnemyAbility>(static_cast<std::uint32_t>(a)|static_cast<std::uint32_t>(b));
}
constexpr bool hasAbility(EnemyAbility set,EnemyAbility flag) {
    return (static_cast<std::uint32_t>(set)&static_cast<std::uint32_t>(flag))!=0;
}

enum class EnemyVariant : std::uint8_t {
    Base = 0,
    ScavengerRagpicker,
    ScavengerFeral,
    ScavengerCarrion,
    ScavengerScuttler,
    ScavengerBlightfed,
    ReaverChainbound,
    ReaverSlagfist,
    ReaverHollowed,
    ReaverYokebreaker,
    ReaverGrindmaw,
    WhisperAshling,
    WhisperNightcut,
    WhisperVeilwalk,
    WhisperGutterghost,
    WhisperMourner,
    DronePatternOne,
    DroneInterdictor,
    DroneLancer,
    DroneRelay,
    DroneKillSwitch,
    LictorSanctioned,
    LictorAegis,
    LictorCensor,
    LictorCustodian,
    LictorInquisitor,
    AdeptAcolyte,
    AdeptVivifier,
    AdeptResonant,
    AdeptMarshal,
    AdeptNullSpeaker
};

struct EnemyProfile {
    EnemyArchetype archetype{EnemyArchetype::Drone};
    EnemyFaction faction{EnemyFaction::Empire};
    EnemyRank rank{EnemyRank::Standard};
    std::string_view name;
    std::string_view combatVerb;
    float maxHealth{};
    float moveSpeed{};
    float attackDamage{};
    float attackRange{};
    float attackPeriod{};
    float supportRange{};
    float supportHeal{};
    float armorRating{};
    CombatElement element{CombatElement::Inert};
    float resilience{};
    float dodgeChance{};
    TerrainDamageCategory terrainAttack{TerrainDamageCategory::MarkOnly};
};


struct EnemyBossProfile {
    EnemyArchetype archetype{EnemyArchetype::Praetor};
    std::string_view name;
    int phaseCount{};
    bool immovable{};
    bool fireImmune{};
    int summonsPerPhaseMultiplier{};
    float crowdTricklePerPhase{};
    float phaseOneShieldFraction{};
    float shieldRegenFractionPerTick{};
    float finalPhaseDamageMultiplier{1.0f};
    float finalPhaseReflectionShare{};
};

struct EnemyVariantProfile {
    EnemyVariant variant{EnemyVariant::Base};
    EnemyArchetype archetype{EnemyArchetype::Drone};
    std::string_view name;
    float healthMultiplier{1.0f};
    float damageMultiplier{1.0f};
    float speedMultiplier{1.0f};
    EnemyAbility abilities{EnemyAbility::None};
};

const char* enemyArchetypeName(EnemyArchetype archetype);
const EnemyProfile& enemyProfile(EnemyArchetype archetype);
const EnemyVariantProfile& enemyVariantProfile(EnemyVariant variant);
const EnemyBossProfile* enemyBossProfile(EnemyArchetype archetype);
EnemyProfile resolvedEnemyProfile(EnemyArchetype archetype, EnemyVariant variant = EnemyVariant::Base);

} // namespace elysium
