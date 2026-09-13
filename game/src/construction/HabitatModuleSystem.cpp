#include "construction/HabitatModuleSystem.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Manage modular sealed habitat pieces, interfaces, pressure zones, service ports, and construction state.
bool HabitatModuleSystemStore::apply(const HabitatModuleSystemOp& o) { if(!o.id) return false; auto& d=data_[o.id]; d.revision=revision_++; d.id=o.id; d.owner=o.owner; d.ref=o.ref; d.value=o.value; d.flags=o.flags; d.live=true; return true; }
bool HabitatModuleSystemStore::erase(std::uint64_t id) { return data_.erase(id)!=0; }
const HabitatModuleSystemData* HabitatModuleSystemStore::find(std::uint64_t id) const { auto it=data_.find(id); return it==data_.end()?nullptr:&it->second; }
std::vector<HabitatModuleSystemData> HabitatModuleSystemStore::snapshot() const { std::vector<HabitatModuleSystemData> v; v.reserve(data_.size()); for(auto& [id,d]:data_) if(d.live) v.push_back(d); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.id<b.id;}); return v; }
void HabitatModuleSystemStore::clear() { data_.clear(); revision_=1; }
}
