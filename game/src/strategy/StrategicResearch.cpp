#include "strategy/StrategicResearch.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Allocate civilization research capacity across technology domains, strategic needs, captured tech, and institutions.
bool StrategicResearchStore::apply(const StrategicResearchOp& o) { if(!o.id) return false; auto& d=data_[o.id]; d.revision=revision_++; d.id=o.id; d.owner=o.owner; d.ref=o.ref; d.value=o.value; d.flags=o.flags; d.live=true; return true; }
bool StrategicResearchStore::erase(std::uint64_t id) { return data_.erase(id)!=0; }
const StrategicResearchData* StrategicResearchStore::find(std::uint64_t id) const { auto it=data_.find(id); return it==data_.end()?nullptr:&it->second; }
std::vector<StrategicResearchData> StrategicResearchStore::snapshot() const { std::vector<StrategicResearchData> v; v.reserve(data_.size()); for(auto& [id,d]:data_) if(d.live) v.push_back(d); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.id<b.id;}); return v; }
void StrategicResearchStore::clear() { data_.clear(); revision_=1; }
}
