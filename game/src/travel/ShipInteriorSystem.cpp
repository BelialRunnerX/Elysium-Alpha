#include "travel/ShipInteriorSystem.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Represent ship compartment graphs, pressure zones, doors, utilities, damage, crew stations, and boarding topology.
bool ShipInteriorSystemSystem::submit(const ShipInteriorSystemCommand& c) { if(!c.subject) return false; auto&s=map_[c.subject]; s.revision=revision_++; s.subject=c.subject; s.owner=c.owner; s.target=c.target; s.strength=c.strength; s.mode=c.mode; s.active=true; return true; }
const ShipInteriorSystemState* ShipInteriorSystemSystem::find(std::uint64_t id) const { auto it=map_.find(id); return it==map_.end()?nullptr:&it->second; }
std::vector<ShipInteriorSystemState> ShipInteriorSystemSystem::states() const { std::vector<ShipInteriorSystemState> v; v.reserve(map_.size()); for(const auto&[id,s]:map_) if(s.active) v.push_back(s); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.subject<b.subject;}); return v; }
bool ShipInteriorSystemSystem::cancel(std::uint64_t id) { return map_.erase(id)!=0; }
void ShipInteriorSystemSystem::clear() { map_.clear(); revision_=1; }
}
