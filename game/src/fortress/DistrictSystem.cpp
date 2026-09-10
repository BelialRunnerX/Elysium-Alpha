#include "fortress/DistrictSystem.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Partition settlements into sparse functional districts with stable identities, services, policies, and expansion boundaries.
bool DistrictSystemStore::apply(const DistrictSystemOp& o) { if(!o.id) return false; auto& d=data_[o.id]; d.revision=revision_++; d.id=o.id; d.owner=o.owner; d.ref=o.ref; d.value=o.value; d.flags=o.flags; d.live=true; return true; }
bool DistrictSystemStore::erase(std::uint64_t id) { return data_.erase(id)!=0; }
const DistrictSystemData* DistrictSystemStore::find(std::uint64_t id) const { auto it=data_.find(id); return it==data_.end()?nullptr:&it->second; }
std::vector<DistrictSystemData> DistrictSystemStore::snapshot() const { std::vector<DistrictSystemData> v; v.reserve(data_.size()); for(auto& [id,d]:data_) if(d.live) v.push_back(d); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.id<b.id;}); return v; }
void DistrictSystemStore::clear() { data_.clear(); revision_=1; }
}
