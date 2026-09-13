#include "terraform/TerraformingRisk.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Aggregate runaway greenhouse, ecosystem collapse, invasive species, toxicity, and infrastructure failure risk.
bool TerraformingRiskStore::apply(const TerraformingRiskOp& o) { if(!o.id) return false; auto& d=data_[o.id]; d.revision=revision_++; d.id=o.id; d.owner=o.owner; d.ref=o.ref; d.value=o.value; d.flags=o.flags; d.live=true; return true; }
bool TerraformingRiskStore::erase(std::uint64_t id) { return data_.erase(id)!=0; }
const TerraformingRiskData* TerraformingRiskStore::find(std::uint64_t id) const { auto it=data_.find(id); return it==data_.end()?nullptr:&it->second; }
std::vector<TerraformingRiskData> TerraformingRiskStore::snapshot() const { std::vector<TerraformingRiskData> v; v.reserve(data_.size()); for(auto& [id,d]:data_) if(d.live) v.push_back(d); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.id<b.id;}); return v; }
void TerraformingRiskStore::clear() { data_.clear(); revision_=1; }
}
