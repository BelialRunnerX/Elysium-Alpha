// Intended function: imported combat implementation for CombatRules; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include <array>
#include <cstdint>

namespace elysium {

enum class CombatElement : std::uint8_t {
    Inert = 0,
    Void = 1,
    Plasma = 2,
    Neural = 3,
    Dimensional = 4,
    Kinetic = 5
};

enum class DamageKind : std::uint8_t {
    Melee = 0,
    Ranged = 1,
    Fire = 2,
    Explosion = 3,
    Hazard = 4,
    Impact = 5,
    Reflection = 6
};

enum class TerrainDamageCategory : std::uint8_t {
    None = 0,
    MarkOnly = 1,
    FragileMicrodetail = 2,
    Excavation = 3,
    StructuralBreach = 4
};

enum class DamagePipelineStage : std::uint8_t {
    DodgeInvulnerability = 0,
    SpecialMitigation = 1,
    ElementRelationship = 2,
    AttackScaling = 3,
    Critical = 4,
    DefenseResilience = 5,
    Reflection = 6,
    Lifesteal = 7,
    FinalEvent = 8
};

struct CombatantModifiers {
    bool invulnerable{};
    float dodgeRune{};
    float dodgeReflex{};
    float dodgePassive{};
    float heatReduction{};
    CombatElement element{CombatElement::Inert};
    int armorTier{};
    float psionicScale{1.0f};
    float damageReduction{};
    float defenceScale{1.0f};
    float reflectShare{};
};

struct AttackModifiers {
    DamageKind kind{DamageKind::Ranged};
    bool piercesInvulnerability{};
    TerrainDamageCategory terrainCategory{TerrainDamageCategory::None};
    CombatElement element{CombatElement::Inert};
    int weaponTier{};
    float psionicScale{1.0f};
    float meleeBaseDamage{};
    float weaponMultiplier{1.0f};
    float attackScale{1.0f};
    float criticalChance{};
    float criticalMultiplier{1.5f};
    float lifestealShare{};
};

struct DamageContext {
    std::uint64_t eventId{};
    std::uint64_t attackerStableId{};
    std::uint64_t defenderStableId{};
    float baseAmount{};
    AttackModifiers attack{};
    CombatantModifiers defender{};
    bool reflecting{};
};

struct DamageEvent {
    std::uint64_t eventId{};
    std::uint64_t attackerStableId{};
    std::uint64_t defenderStableId{};
    DamageKind kind{DamageKind::Ranged};
    TerrainDamageCategory terrainCategory{TerrainDamageCategory::None};
    float landed{};
    float reflected{};
    float lifesteal{};
    bool dodged{};
    bool critical{};
    bool reflection{};
};

struct DamageResolution {
    float incoming{};
    float afterSpecialMitigation{};
    float afterElement{};
    float afterAttackScaling{};
    float afterCritical{};
    float landed{};
    float reflected{};
    float lifesteal{};
    float dodgeChance{};
    bool invulnerable{};
    bool dodged{};
    bool critical{};
    bool reflectionSuppressed{};
    DamageEvent event{};
    std::array<DamagePipelineStage,9> stageOrder{};
};

// Independent percentage shares combine without reaching immunity from ordinary
// stacking. Inputs are clamped to [0,1].
float combineShares(float a, float b, float c = 0.0f);

// Tier advantage follows the inherited gear table for tiers 0..5 and extends
// beyond it asymptotically so unbounded ascension never creates a hard clamp.
float tierAdvantage(int tier);

bool elementBeats(CombatElement attacker, CombatElement defender);

// Pure deterministic damage calculation. This function never mutates actors.
// The caller applies landed/reflected/lifesteal exactly once in its owner/commit
// phase and publishes the returned DamageEvent.
DamageResolution resolveDamage(const DamageContext& context);

} // namespace elysium
