#include "travel/ShipAtmosphereSystem.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Track compartment oxygen, pressure, contaminants, leaks, vents, scrubbers, fires, and emergency isolation aboard ships.
bool ShipAtmosphereSystemSystem::submit(const ShipAtmosphereSystemCommand& c) { if(!c.subject) return false; auto&s=map_[c.subject]; s.revision=revision_++; s.subject=c.subject; s.owner=c.owner; s.target=c.target; s.strength=c.strength; s.mode=c.mode; s.active=true; return true; }
const ShipAtmosphereSystemState* ShipAtmosphereSystemSystem::find(std::uint64_t id) const { auto it=map_.find(id); return it==map_.end()?nullptr:&it->second; }
std::vector<ShipAtmosphereSystemState> ShipAtmosphereSystemSystem::states() const { std::vector<ShipAtmosphereSystemState> v; v.reserve(map_.size()); for(const auto&[id,s]:map_) if(s.active) v.push_back(s); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.subject<b.subject;}); return v; }
bool ShipAtmosphereSystemSystem::cancel(std::uint64_t id) { return map_.erase(id)!=0; }
void ShipAtmosphereSystemSystem::clear() { map_.clear(); revision_=1; }
}
