#include "history/TechnologyHistorySystem.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Track discoveries, inventors, institutions, patents/ownership hooks, diffusion, accidents, and strategic impact.
bool TechnologyHistorySystemStore::apply(const TechnologyHistorySystemOp& o) { if(!o.id) return false; auto& d=data_[o.id]; d.revision=revision_++; d.id=o.id; d.owner=o.owner; d.ref=o.ref; d.value=o.value; d.flags=o.flags; d.live=true; return true; }
bool TechnologyHistorySystemStore::erase(std::uint64_t id) { return data_.erase(id)!=0; }
const TechnologyHistorySystemData* TechnologyHistorySystemStore::find(std::uint64_t id) const { auto it=data_.find(id); return it==data_.end()?nullptr:&it->second; }
std::vector<TechnologyHistorySystemData> TechnologyHistorySystemStore::snapshot() const { std::vector<TechnologyHistorySystemData> v; v.reserve(data_.size()); for(auto& [id,d]:data_) if(d.live) v.push_back(d); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.id<b.id;}); return v; }
void TechnologyHistorySystemStore::clear() { data_.clear(); revision_=1; }
}
