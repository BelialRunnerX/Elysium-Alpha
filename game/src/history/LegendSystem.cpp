#include "history/LegendSystem.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Promote selected historical events/people/artifacts into mutable legends with retellings, distortion, and cultural reach.
bool LegendSystemStore::apply(const LegendSystemOp& o) { if(!o.id) return false; auto& d=data_[o.id]; d.revision=revision_++; d.id=o.id; d.owner=o.owner; d.ref=o.ref; d.value=o.value; d.flags=o.flags; d.live=true; return true; }
bool LegendSystemStore::erase(std::uint64_t id) { return data_.erase(id)!=0; }
const LegendSystemData* LegendSystemStore::find(std::uint64_t id) const { auto it=data_.find(id); return it==data_.end()?nullptr:&it->second; }
std::vector<LegendSystemData> LegendSystemStore::snapshot() const { std::vector<LegendSystemData> v; v.reserve(data_.size()); for(auto& [id,d]:data_) if(d.live) v.push_back(d); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.id<b.id;}); return v; }
void LegendSystemStore::clear() { data_.clear(); revision_=1; }
}
