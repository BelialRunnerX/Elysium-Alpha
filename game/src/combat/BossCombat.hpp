// Intended function: imported combat implementation for BossCombat; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "combat/EnemyCatalog.hpp"

namespace elysium {

struct BossPhaseEvaluation {
    int phase{1};
    bool immovable{};
    bool fireImmune{};
    float damageMultiplier{1.0f};
    float reflectionShare{};
    float shieldCapacity{};
    float shieldRegenFractionPerTick{};
    int summonsOnAdvance{};
    int crowdCap{};
    float crowdTrickleChancePerTick{};
    bool speedIncreasesWithPhase{};
};

// Inherited authored boss scaling: ordinary scaled max health is multiplied by
// 2.0 + 0.6 * sqrt(level - 1). Level is clamped to at least 1.
float bossHealthMultiplier(int level);

// Evenly-spaced health thresholds, with phases numbered from 1. currentPhase
// enforces the inherited "phase transitions only move forward" contract.
int bossPhaseForHealth(EnemyArchetype archetype, float health, float maxHealth, int currentPhase = 1);

// Pure data evaluation of the authored Choir/Praetor phase mechanics. The
// caller owns mutation, spawning, shield state, presentation, and persistence.
BossPhaseEvaluation evaluateBossPhase(EnemyArchetype archetype, float health, float maxHealth,
                                      int currentPhase = 1);

} // namespace elysium
