#include "combat/VehicleWeaponSystem.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Resolve mounted ballistic, beam, missile, mining, point-defense, and utility weapons through common combat events.
bool VehicleWeaponSystemSystem::submit(const VehicleWeaponSystemCommand& c) { if(!c.subject) return false; auto&s=map_[c.subject]; s.revision=revision_++; s.subject=c.subject; s.owner=c.owner; s.target=c.target; s.strength=c.strength; s.mode=c.mode; s.active=true; return true; }
const VehicleWeaponSystemState* VehicleWeaponSystemSystem::find(std::uint64_t id) const { auto it=map_.find(id); return it==map_.end()?nullptr:&it->second; }
std::vector<VehicleWeaponSystemState> VehicleWeaponSystemSystem::states() const { std::vector<VehicleWeaponSystemState> v; v.reserve(map_.size()); for(const auto&[id,s]:map_) if(s.active) v.push_back(s); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.subject<b.subject;}); return v; }
bool VehicleWeaponSystemSystem::cancel(std::uint64_t id) { return map_.erase(id)!=0; }
void VehicleWeaponSystemSystem::clear() { map_.clear(); revision_=1; }
}
