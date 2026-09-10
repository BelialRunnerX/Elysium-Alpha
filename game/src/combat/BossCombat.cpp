// Intended function: imported combat implementation for BossCombat; preserves the agent-authored subsystem contract for later integration/debugging.
#include "combat/BossCombat.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace elysium {

float bossHealthMultiplier(int level) {
    const int clamped=std::max(1,level);
    return 2.0f+0.6f*std::sqrt(static_cast<float>(clamped-1));
}

int bossPhaseForHealth(EnemyArchetype archetype,float health,float maxHealth,int currentPhase) {
    const auto* boss=enemyBossProfile(archetype);
    if(!boss) throw std::invalid_argument("enemy archetype is not an authored boss");
    const int clampedCurrent=std::clamp(currentPhase,1,boss->phaseCount);
    const float fraction=maxHealth>0.0f?std::clamp(health/maxHealth,0.0f,1.0f):0.0f;
    int computed=1;
    for(int phase=2;phase<=boss->phaseCount;++phase) {
        const float threshold=static_cast<float>(boss->phaseCount-phase+1)/static_cast<float>(boss->phaseCount);
        if(fraction<=threshold) computed=phase;
    }
    return std::max(clampedCurrent,computed);
}

BossPhaseEvaluation evaluateBossPhase(EnemyArchetype archetype,float health,float maxHealth,int currentPhase) {
    const auto* boss=enemyBossProfile(archetype);
    if(!boss) throw std::invalid_argument("enemy archetype is not an authored boss");
    BossPhaseEvaluation out{};
    out.phase=bossPhaseForHealth(archetype,health,maxHealth,currentPhase);
    out.immovable=boss->immovable;
    out.fireImmune=boss->fireImmune;
    out.speedIncreasesWithPhase=(archetype==EnemyArchetype::Choir);

    if(archetype==EnemyArchetype::Praetor) {
        if(out.phase==1) {
            out.shieldCapacity=std::max(0.0f,maxHealth)*boss->phaseOneShieldFraction;
            out.shieldRegenFractionPerTick=boss->shieldRegenFractionPerTick;
        } else {
            out.damageMultiplier=boss->finalPhaseDamageMultiplier;
            out.reflectionShare=boss->finalPhaseReflectionShare;
        }
    } else if(archetype==EnemyArchetype::Choir) {
        out.crowdCap=4+out.phase*boss->summonsPerPhaseMultiplier;
        out.crowdTrickleChancePerTick=boss->crowdTricklePerPhase*static_cast<float>(out.phase);
        if(out.phase>currentPhase) out.summonsOnAdvance=out.phase*boss->summonsPerPhaseMultiplier;
    }
    return out;
}

} // namespace elysium
