#include "construction/TerraformProjectSystem.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Track long-duration terraforming projects, required infrastructure, resource budgets, milestones, and ecological consequences.
bool TerraformProjectSystemStore::apply(const TerraformProjectSystemOp& o) { if(!o.id) return false; auto& d=data_[o.id]; d.revision=revision_++; d.id=o.id; d.owner=o.owner; d.ref=o.ref; d.value=o.value; d.flags=o.flags; d.live=true; return true; }
bool TerraformProjectSystemStore::erase(std::uint64_t id) { return data_.erase(id)!=0; }
const TerraformProjectSystemData* TerraformProjectSystemStore::find(std::uint64_t id) const { auto it=data_.find(id); return it==data_.end()?nullptr:&it->second; }
std::vector<TerraformProjectSystemData> TerraformProjectSystemStore::snapshot() const { std::vector<TerraformProjectSystemData> v; v.reserve(data_.size()); for(auto& [id,d]:data_) if(d.live) v.push_back(d); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.id<b.id;}); return v; }
void TerraformProjectSystemStore::clear() { data_.clear(); revision_=1; }
}
