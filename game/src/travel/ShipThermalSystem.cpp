#include "travel/ShipThermalSystem.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Track heat generation, coolant loops, radiators, sinks, compartment temperature, and weapon/reactor thermal limits.
bool ShipThermalSystemSystem::submit(const ShipThermalSystemCommand& c) { if(!c.subject) return false; auto&s=map_[c.subject]; s.revision=revision_++; s.subject=c.subject; s.owner=c.owner; s.target=c.target; s.strength=c.strength; s.mode=c.mode; s.active=true; return true; }
const ShipThermalSystemState* ShipThermalSystemSystem::find(std::uint64_t id) const { auto it=map_.find(id); return it==map_.end()?nullptr:&it->second; }
std::vector<ShipThermalSystemState> ShipThermalSystemSystem::states() const { std::vector<ShipThermalSystemState> v; v.reserve(map_.size()); for(const auto&[id,s]:map_) if(s.active) v.push_back(s); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.subject<b.subject;}); return v; }
bool ShipThermalSystemSystem::cancel(std::uint64_t id) { return map_.erase(id)!=0; }
void ShipThermalSystemSystem::clear() { map_.clear(); revision_=1; }
}
