// Intended function: imported combat implementation for EnemyAiPlanner; preserves the agent-authored subsystem contract for later integration/debugging.
#include "combat/EnemyAiPlanner.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>

namespace elysium {

EnemyAiPlanResult planAuthoredEnemyAi(JobSystem& jobs,float dt,Vec3 playerCenter,
                                      std::vector<EnemyAiActorSnapshot> actors) {
    using Clock=std::chrono::steady_clock;
    using Ns=std::chrono::nanoseconds;
    EnemyAiPlanResult out{};
    dt=std::max(0.0f,dt);
    std::sort(actors.begin(),actors.end(),[](const auto& a,const auto& b){return a.stableId<b.stableId;});
    out.actors=std::move(actors);
    out.senses.resize(out.actors.size());
    out.intents.resize(out.actors.size());

    auto start=Clock::now();
    jobs.parallelFor(out.actors.size(),[&](std::size_t i) {
        const auto& actor=out.actors[i];
        const Vec3 up=lengthSq(actor.position)>0.001f?normalize(actor.position):Vec3{0,1,0};
        const Vec3 toPlayer=playerCenter-actor.position;
        const float radial=dot(toPlayer,up);
        const Vec3 tangent=toPlayer-up*radial;
        EnemyAiSense sense{};
        sense.tangentDistance=length(tangent);
        sense.spatialDistance=length(toPlayer);
        if(sense.tangentDistance>0.01f) sense.tangentDirection=tangent/sense.tangentDistance;
        out.senses[i]=sense;
    },8);
    out.senseNanoseconds=static_cast<std::uint64_t>(std::chrono::duration_cast<Ns>(Clock::now()-start).count());

    start=Clock::now();
    jobs.parallelFor(out.actors.size(),[&](std::size_t i) {
        const auto& actor=out.actors[i];
        const auto& sense=out.senses[i];
        EnemyAiIntent intent{};
        intent.actorStableId=actor.stableId;
        intent.destroy=actor.health<=0.0f;
        intent.nextCooldown=std::max(0.0f,actor.cooldown-dt);
        if(!intent.destroy) {
            intent.velocity=sense.tangentDirection*actor.moveSpeed;
            intent.move=!actor.immovable && sense.tangentDistance>1.8f;
            intent.attack=sense.spatialDistance<actor.attackRange && intent.nextCooldown<=0.0f;
            if(intent.attack) intent.nextCooldown=actor.attackPeriod;

            if(actor.supportHeal>0.0f && actor.supportRange>0.0f && actor.cooldown<=0.0f) {
                float bestFraction=1.0f;
                std::uint64_t bestId=0;
                for(const auto& ally:out.actors) {
                    if(ally.stableId==actor.stableId || ally.health<=0.0f || ally.health>=ally.maxHealth) continue;
                    if(lengthSq(ally.position-actor.position)>actor.supportRange*actor.supportRange) continue;
                    const float fraction=ally.maxHealth>0.0f?ally.health/ally.maxHealth:1.0f;
                    if(fraction<bestFraction || (std::abs(fraction-bestFraction)<1e-6f && (bestId==0 || ally.stableId<bestId))) {
                        bestFraction=fraction;
                        bestId=ally.stableId;
                    }
                }
                if(bestId!=0) {
                    intent.supportTargetStableId=bestId;
                    intent.supportHeal=actor.supportHeal;
                    intent.attack=false;
                    intent.nextCooldown=std::max(intent.nextCooldown,2.2f);
                }
            }
        }
        out.intents[i]=intent;
    },8);
    out.thinkNanoseconds=static_cast<std::uint64_t>(std::chrono::duration_cast<Ns>(Clock::now()-start).count());
    return out;
}

} // namespace elysium
