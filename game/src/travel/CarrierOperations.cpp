#include "travel/CarrierOperations.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Track embarked craft, pilots, launch/recovery cycles, maintenance, fuel, ammunition, and sortie planning.
bool CarrierOperationsSystem::submit(const CarrierOperationsCommand& c) { if(!c.subject) return false; auto&s=map_[c.subject]; s.revision=revision_++; s.subject=c.subject; s.owner=c.owner; s.target=c.target; s.strength=c.strength; s.mode=c.mode; s.active=true; return true; }
const CarrierOperationsState* CarrierOperationsSystem::find(std::uint64_t id) const { auto it=map_.find(id); return it==map_.end()?nullptr:&it->second; }
std::vector<CarrierOperationsState> CarrierOperationsSystem::states() const { std::vector<CarrierOperationsState> v; v.reserve(map_.size()); for(const auto&[id,s]:map_) if(s.active) v.push_back(s); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.subject<b.subject;}); return v; }
bool CarrierOperationsSystem::cancel(std::uint64_t id) { return map_.erase(id)!=0; }
void CarrierOperationsSystem::clear() { map_.clear(); revision_=1; }
}
