#include "travel/LaunchWindowPlanner.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Compute deterministic surface-to-orbit and interbody launch windows from simplified orbital summaries.
bool LaunchWindowPlannerStore::apply(const LaunchWindowPlannerOp& o) { if(!o.id) return false; auto& d=data_[o.id]; d.revision=revision_++; d.id=o.id; d.owner=o.owner; d.ref=o.ref; d.value=o.value; d.flags=o.flags; d.live=true; return true; }
bool LaunchWindowPlannerStore::erase(std::uint64_t id) { return data_.erase(id)!=0; }
const LaunchWindowPlannerData* LaunchWindowPlannerStore::find(std::uint64_t id) const { auto it=data_.find(id); return it==data_.end()?nullptr:&it->second; }
std::vector<LaunchWindowPlannerData> LaunchWindowPlannerStore::snapshot() const { std::vector<LaunchWindowPlannerData> v; v.reserve(data_.size()); for(auto& [id,d]:data_) if(d.live) v.push_back(d); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.id<b.id;}); return v; }
void LaunchWindowPlannerStore::clear() { data_.clear(); revision_=1; }
}
