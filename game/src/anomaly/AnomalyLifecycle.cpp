#include "anomaly/AnomalyLifecycle.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Advance anomaly activation, stability, hazards, rewards, propagation, and collapse from deterministic state.
bool AnomalyLifecycleModel::update(const AnomalyLifecycleInput& input) { if(input.keyId==0) return false; auto& s=data_[input.keyId]; s.revision=revision_++; s.ownerId=input.ownerId; s.keyId=input.keyId; s.amount=input.amount; s.policy=input.policy; s.valid=true; return true; }
const AnomalyLifecycleSnapshot* AnomalyLifecycleModel::get(std::uint64_t keyId) const { auto it=data_.find(keyId); return it==data_.end()?nullptr:&it->second; }
std::vector<AnomalyLifecycleSnapshot> AnomalyLifecycleModel::ordered() const { std::vector<AnomalyLifecycleSnapshot> out; out.reserve(data_.size()); for(const auto& [id,s]:data_) if(s.valid) out.push_back(s); std::sort(out.begin(),out.end(),[](const auto&a,const auto&b){return a.keyId<b.keyId;}); return out; }
void AnomalyLifecycleModel::clear() { data_.clear(); revision_=1; }
}
