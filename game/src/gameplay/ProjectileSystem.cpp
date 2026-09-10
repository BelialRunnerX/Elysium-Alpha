// Intended function: maintain deterministic projectile lifetime/order and remove expired/hit projectiles after bounded movement steps.
#include "gameplay/ProjectileSystem.hpp"
#include <algorithm>
namespace elysium{void ProjectileSystem::spawn(ProjectileState p){if(!p.projectileId||p.remainingSeconds<=0)return;projectiles_.push_back(p);std::sort(projectiles_.begin(),projectiles_.end(),[](auto&a,auto&b){return a.projectileId<b.projectileId;});}void ProjectileSystem::compact(){projectiles_.erase(std::remove_if(projectiles_.begin(),projectiles_.end(),[](auto&p){return p.remainingSeconds<=0;}),projectiles_.end());}}
