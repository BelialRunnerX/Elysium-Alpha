#include "warfare/FleetDoctrineSystem.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Store faction fleet doctrine preferences for formation, engagement, retreat, escort, interdiction, and resupply.
bool FleetDoctrineSystemStore::apply(const FleetDoctrineSystemOp& o) { if(!o.id) return false; auto& d=data_[o.id]; d.revision=revision_++; d.id=o.id; d.owner=o.owner; d.ref=o.ref; d.value=o.value; d.flags=o.flags; d.live=true; return true; }
bool FleetDoctrineSystemStore::erase(std::uint64_t id) { return data_.erase(id)!=0; }
const FleetDoctrineSystemData* FleetDoctrineSystemStore::find(std::uint64_t id) const { auto it=data_.find(id); return it==data_.end()?nullptr:&it->second; }
std::vector<FleetDoctrineSystemData> FleetDoctrineSystemStore::snapshot() const { std::vector<FleetDoctrineSystemData> v; v.reserve(data_.size()); for(auto& [id,d]:data_) if(d.live) v.push_back(d); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.id<b.id;}); return v; }
void FleetDoctrineSystemStore::clear() { data_.clear(); revision_=1; }
}
