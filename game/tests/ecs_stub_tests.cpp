// Production contract probe for the authoritative EnTT-backed ECS surface.
#include "ecs/EcsWorld.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace elysium;

namespace {

void require(bool condition,const std::string& message) {
    if(!condition) throw std::runtime_error(message);
}

EnemySnapshot findEnemy(const EcsWorld& ecs,std::uint64_t stableId) {
    for(const auto& e:ecs.enemies()) if(e.stableId==stableId) return e;
    throw std::runtime_error("enemy stable ID not found");
}

struct ScenarioResult {
    PlayerSnapshot player;
    std::vector<EnemySnapshot> enemies;
    int supportActions{};
};

ScenarioResult runScenario(JobSystem& jobs) {
    EcsWorld ecs;
    ecs.createPlayer({5.0f,0.0f,10.0f});
    ecs.spawnEnemy({0.5f,0.0f,10.0f},EnemyArchetype::Lictor,10,ActorDomain::PlanetSurface);
    ecs.spawnEnemy({0.0f,0.0f,10.0f},EnemyArchetype::Adept,20,ActorDomain::PlanetSurface);
    ecs.spawnEnemy({1.0f,0.0f,10.0f},EnemyArchetype::Drone,30,ActorDomain::PlanetSurface);

    // Command-buffer structural mutation is owner-thread committed.
    ecs.queueSpawnEnemy({8.0f,0.0f,10.0f},EnemyArchetype::Praetor,40,ActorDomain::PlanetSurface);
    const auto spawnFlush=ecs.flushCommands();
    require(spawnFlush.spawned==1 && ecs.enemies().size()==4,
            "queued enemy spawn did not commit exactly once");

    // Damage commands are intentionally ordered by command sequence in the
    // surviving runtime contract. Two queued commands therefore apply twice.
    const float before=findEnemy(ecs,10).health;
    ecs.queueDamageEnemy(10,20.0f);
    ecs.queueDamageEnemy(10,20.0f);
    const auto damageFlush=ecs.flushCommands();
    const float after=findEnemy(ecs,10).health;
    require(damageFlush.damaged==2 && after==before-40.0f,
            "ordered command-buffer damage did not match the authoritative contract");

    auto hover=[](Vec3 p){return p;};
    auto path=[](Vec3 position,Vec3 velocity,float dt){return position+velocity*dt;};
    // First tick expires the initial cooldown; the Adept may then publish a
    // support action through the phased deterministic AI path.
    (void)ecs.updateSurfaceEnemiesPhased(jobs,0.31f,{5.0f,0.0f,10.0f},hover,path);
    const auto telemetry=ecs.updateSurfaceEnemiesPhased(jobs,0.01f,{5.0f,0.0f,10.0f},hover,path);

    auto enemies=ecs.enemies();
    std::sort(enemies.begin(),enemies.end(),[](const auto& a,const auto& b){return a.stableId<b.stableId;});
    return {ecs.player(),std::move(enemies),telemetry.supportActions};
}

void testEcsCombatContractAcrossWorkers() {
    JobSystem serial(SerialJobs);
    const auto baseline=runScenario(serial);
    for(std::size_t workers : {1u,2u,4u,8u}) {
        JobSystem jobs(workers);
        const auto result=runScenario(jobs);
        require(result.enemies.size()==baseline.enemies.size(),"worker count changed enemy population");
        require(result.player.health==baseline.player.health,"worker count changed player combat outcome");
        require(result.supportActions==baseline.supportActions,"worker count changed support decision count");
        for(std::size_t i=0;i<result.enemies.size();++i) {
            const auto& a=result.enemies[i];
            const auto& b=baseline.enemies[i];
            require(a.stableId==b.stableId && a.health==b.health &&
                    a.position.x==b.position.x && a.position.y==b.position.y && a.position.z==b.position.z &&
                    a.archetype==b.archetype && a.domain==b.domain,
                    "worker count changed deterministic ECS enemy state");
        }
    }
}

void testRayCombatAndCommandDeath() {
    EcsWorld ecs;
    ecs.createPlayer({0.0f,0.0f,0.0f});
    ecs.spawnEnemy({0.0f,0.0f,10.0f},EnemyArchetype::Praetor,77,ActorDomain::Planar);
    const auto before=findEnemy(ecs,77).health;
    const auto hit=ecs.attackRay({0.0f,0.45f,0.0f},{0.0f,0.0f,1.0f},20.0f,12.0f,ActorDomain::Planar);
    require(hit.hit && hit.stableId==77 && !hit.killed,"direct ray attack missed deterministic target");
    require(findEnemy(ecs,77).health==before-12.0f,"ray attack did not apply requested damage");

    ecs.queueDamageEnemy(77,10000.0f);
    const auto flush=ecs.flushCommands();
    require(flush.damaged==1 && flush.destroyed==1 && ecs.enemies().empty(),
            "lethal command did not destroy the target during owner-thread commit");
}

} // namespace

int main() {
    try {
        testEcsCombatContractAcrossWorkers();
        testRayCombatAndCommandDeath();
        std::cout << "Elysium ECS production contract tests: PASS\n";
        return 0;
    } catch(const std::exception& e) {
        std::cerr << "Elysium ECS production contract tests: FAIL: " << e.what() << '\n';
        return 1;
    }
}
