// Intended function: imported combat implementation for EnemyAiPlanner; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "combat/EnemyCatalog.hpp"
#include "core/JobSystem.hpp"
#include "core/Math.hpp"

#include <cstdint>
#include <vector>

namespace elysium {

struct EnemyAiActorSnapshot {
    std::uint64_t stableId{};
    Vec3 position{};
    float health{};
    float maxHealth{};
    float cooldown{};
    EnemyArchetype archetype{EnemyArchetype::Drone};
    float moveSpeed{};
    float attackDamage{};
    float attackRange{};
    float attackPeriod{};
    float supportRange{};
    float supportHeal{};
    CombatElement element{CombatElement::Inert};
    float resilience{};
    float dodgeChance{};
    TerrainDamageCategory terrainAttack{TerrainDamageCategory::MarkOnly};
    bool immovable{};
};

struct EnemyAiSense {
    Vec3 tangentDirection{};
    float tangentDistance{};
    float spatialDistance{};
};

struct EnemyAiIntent {
    std::uint64_t actorStableId{};
    Vec3 velocity{};
    bool move{};
    bool attack{};
    bool destroy{};
    float nextCooldown{};
    std::uint64_t supportTargetStableId{};
    float supportHeal{};
    CombatElement element{CombatElement::Inert};
    float resilience{};
    float dodgeChance{};
    TerrainDamageCategory terrainAttack{TerrainDamageCategory::MarkOnly};
    bool immovable{};
};

struct EnemyAiPlanResult {
    std::vector<EnemyAiActorSnapshot> actors;
    std::vector<EnemyAiSense> senses;
    std::vector<EnemyAiIntent> intents;
    std::uint64_t senseNanoseconds{};
    std::uint64_t thinkNanoseconds{};
};

// Pure read/compute planner for the authored combat families. Inputs are copied
// and sorted by StableId before parallel work, so result bytes do not depend on
// EnTT iteration order or worker scheduling. No world/ECS mutation occurs here.
EnemyAiPlanResult planAuthoredEnemyAi(JobSystem& jobs, float dt, Vec3 playerCenter,
                                      std::vector<EnemyAiActorSnapshot> actors);

} // namespace elysium
