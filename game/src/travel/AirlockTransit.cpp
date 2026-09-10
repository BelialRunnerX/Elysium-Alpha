#include "travel/AirlockTransit.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Coordinate queued actor/item/vehicle passage through pressure boundaries without owning atmosphere simulation.
bool AirlockTransitStore::apply(const AirlockTransitOp& o) { if(!o.id) return false; auto& d=data_[o.id]; d.revision=revision_++; d.id=o.id; d.owner=o.owner; d.ref=o.ref; d.value=o.value; d.flags=o.flags; d.live=true; return true; }
bool AirlockTransitStore::erase(std::uint64_t id) { return data_.erase(id)!=0; }
const AirlockTransitData* AirlockTransitStore::find(std::uint64_t id) const { auto it=data_.find(id); return it==data_.end()?nullptr:&it->second; }
std::vector<AirlockTransitData> AirlockTransitStore::snapshot() const { std::vector<AirlockTransitData> v; v.reserve(data_.size()); for(auto& [id,d]:data_) if(d.live) v.push_back(d); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.id<b.id;}); return v; }
void AirlockTransitStore::clear() { data_.clear(); revision_=1; }
}
