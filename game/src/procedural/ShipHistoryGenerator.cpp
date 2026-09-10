#include "procedural/ShipHistoryGenerator.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Generate ship construction, owners, voyages, battles, refits, accidents, crew stories, and derelict transitions.
bool ShipHistoryGeneratorStore::apply(const ShipHistoryGeneratorOp& o) { if(!o.id) return false; auto& d=data_[o.id]; d.revision=revision_++; d.id=o.id; d.owner=o.owner; d.ref=o.ref; d.value=o.value; d.flags=o.flags; d.live=true; return true; }
bool ShipHistoryGeneratorStore::erase(std::uint64_t id) { return data_.erase(id)!=0; }
const ShipHistoryGeneratorData* ShipHistoryGeneratorStore::find(std::uint64_t id) const { auto it=data_.find(id); return it==data_.end()?nullptr:&it->second; }
std::vector<ShipHistoryGeneratorData> ShipHistoryGeneratorStore::snapshot() const { std::vector<ShipHistoryGeneratorData> v; v.reserve(data_.size()); for(auto& [id,d]:data_) if(d.live) v.push_back(d); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.id<b.id;}); return v; }
void ShipHistoryGeneratorStore::clear() { data_.clear(); revision_=1; }
}
