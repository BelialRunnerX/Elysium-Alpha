#include "agriculture/PollinationSystem.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Aggregate pollinator activity, wind, greenhouse support, flowering windows, and crop fertility.
bool PollinationSystemModel::update(const PollinationSystemInput& input) { if(input.keyId==0) return false; auto& s=data_[input.keyId]; s.revision=revision_++; s.ownerId=input.ownerId; s.keyId=input.keyId; s.amount=input.amount; s.policy=input.policy; s.valid=true; return true; }
const PollinationSystemSnapshot* PollinationSystemModel::get(std::uint64_t keyId) const { auto it=data_.find(keyId); return it==data_.end()?nullptr:&it->second; }
std::vector<PollinationSystemSnapshot> PollinationSystemModel::ordered() const { std::vector<PollinationSystemSnapshot> out; out.reserve(data_.size()); for(const auto& [id,s]:data_) if(s.valid) out.push_back(s); std::sort(out.begin(),out.end(),[](const auto&a,const auto&b){return a.keyId<b.keyId;}); return out; }
void PollinationSystemModel::clear() { data_.clear(); revision_=1; }
}
