#include "shipboard/ShipEvacuationSystem.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Plan lifeboats, escape pods, suits, rally points, triage, passenger priorities, and abandonment state.
bool ShipEvacuationSystemStore::apply(const ShipEvacuationSystemOp& o) { if(!o.id) return false; auto& d=data_[o.id]; d.revision=revision_++; d.id=o.id; d.owner=o.owner; d.ref=o.ref; d.value=o.value; d.flags=o.flags; d.live=true; return true; }
bool ShipEvacuationSystemStore::erase(std::uint64_t id) { return data_.erase(id)!=0; }
const ShipEvacuationSystemData* ShipEvacuationSystemStore::find(std::uint64_t id) const { auto it=data_.find(id); return it==data_.end()?nullptr:&it->second; }
std::vector<ShipEvacuationSystemData> ShipEvacuationSystemStore::snapshot() const { std::vector<ShipEvacuationSystemData> v; v.reserve(data_.size()); for(auto& [id,d]:data_) if(d.live) v.push_back(d); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.id<b.id;}); return v; }
void ShipEvacuationSystemStore::clear() { data_.clear(); revision_=1; }
}
