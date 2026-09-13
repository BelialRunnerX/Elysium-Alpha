// Intended function: bounded projectile simulation records that emit hit/expiry requests while keeping authoritative damage resolution elsewhere.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium{
struct ProjectileState{std::uint64_t projectileId{},sourceId{},targetId{};float x{},y{},z{},vx{},vy{},vz{},radius{.05f},damage{},remainingSeconds{5};std::uint32_t damageMask{};};
struct ProjectileHitRequest{std::uint64_t projectileId{},sourceId{},targetId{};float x{},y{},z{},damage{};std::uint32_t damageMask{};};
class ProjectileSystem{public:void spawn(ProjectileState p);template<class CollisionFn>std::vector<ProjectileHitRequest>advance(float dt,CollisionFn&&collision){std::vector<ProjectileHitRequest>hits;for(auto&p:projectiles_){if(p.remainingSeconds<=0)continue;float nx=p.x+p.vx*dt,ny=p.y+p.vy*dt,nz=p.z+p.vz*dt;auto target=collision(p,nx,ny,nz);p.x=nx;p.y=ny;p.z=nz;p.remainingSeconds-=dt;if(target){hits.push_back({p.projectileId,p.sourceId,*target,nx,ny,nz,p.damage,p.damageMask});p.remainingSeconds=0;}}compact();return hits;}const std::vector<ProjectileState>&projectiles()const{return projectiles_;}private:void compact();std::vector<ProjectileState>projectiles_;};
}
