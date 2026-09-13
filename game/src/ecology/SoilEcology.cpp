#include "ecology/SoilEcology.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Track soil fertility, moisture, microbes, organic matter, salinity, toxins, erosion, and plant-support capacity.
bool SoilEcologyStore::apply(const SoilEcologyOp& o) { if(!o.id) return false; auto& d=data_[o.id]; d.revision=revision_++; d.id=o.id; d.owner=o.owner; d.ref=o.ref; d.value=o.value; d.flags=o.flags; d.live=true; return true; }
bool SoilEcologyStore::erase(std::uint64_t id) { return data_.erase(id)!=0; }
const SoilEcologyData* SoilEcologyStore::find(std::uint64_t id) const { auto it=data_.find(id); return it==data_.end()?nullptr:&it->second; }
std::vector<SoilEcologyData> SoilEcologyStore::snapshot() const { std::vector<SoilEcologyData> v; v.reserve(data_.size()); for(auto& [id,d]:data_) if(d.live) v.push_back(d); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.id<b.id;}); return v; }
void SoilEcologyStore::clear() { data_.clear(); revision_=1; }
}
