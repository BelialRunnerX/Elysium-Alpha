#include "industry/ChemicalIndustry.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Represent acids, solvents, polymers, fertilizers, medicines, propellants, toxins, and waste-processing production chains.
bool ChemicalIndustryStore::apply(const ChemicalIndustryOp& o) { if(!o.id) return false; auto& d=data_[o.id]; d.revision=revision_++; d.id=o.id; d.owner=o.owner; d.ref=o.ref; d.value=o.value; d.flags=o.flags; d.live=true; return true; }
bool ChemicalIndustryStore::erase(std::uint64_t id) { return data_.erase(id)!=0; }
const ChemicalIndustryData* ChemicalIndustryStore::find(std::uint64_t id) const { auto it=data_.find(id); return it==data_.end()?nullptr:&it->second; }
std::vector<ChemicalIndustryData> ChemicalIndustryStore::snapshot() const { std::vector<ChemicalIndustryData> v; v.reserve(data_.size()); for(auto& [id,d]:data_) if(d.live) v.push_back(d); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.id<b.id;}); return v; }
void ChemicalIndustryStore::clear() { data_.clear(); revision_=1; }
}
