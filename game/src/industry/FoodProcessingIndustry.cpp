#include "industry/FoodProcessingIndustry.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Represent milling, preservation, fermentation, cooking, packaging, refrigeration, nutrition, and spoilage-aware production.
bool FoodProcessingIndustryStore::apply(const FoodProcessingIndustryOp& o) { if(!o.id) return false; auto& d=data_[o.id]; d.revision=revision_++; d.id=o.id; d.owner=o.owner; d.ref=o.ref; d.value=o.value; d.flags=o.flags; d.live=true; return true; }
bool FoodProcessingIndustryStore::erase(std::uint64_t id) { return data_.erase(id)!=0; }
const FoodProcessingIndustryData* FoodProcessingIndustryStore::find(std::uint64_t id) const { auto it=data_.find(id); return it==data_.end()?nullptr:&it->second; }
std::vector<FoodProcessingIndustryData> FoodProcessingIndustryStore::snapshot() const { std::vector<FoodProcessingIndustryData> v; v.reserve(data_.size()); for(auto& [id,d]:data_) if(d.live) v.push_back(d); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.id<b.id;}); return v; }
void FoodProcessingIndustryStore::clear() { data_.clear(); revision_=1; }
}
