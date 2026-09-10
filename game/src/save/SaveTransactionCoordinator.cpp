#include "save/SaveTransactionCoordinator.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Coordinate atomic publication of shard, chunk, stable-object, strategic, and sidecar records through one generation.
bool SaveTransactionCoordinatorModel::update(const SaveTransactionCoordinatorInput& input) { if(input.keyId==0) return false; auto& s=data_[input.keyId]; s.revision=revision_++; s.ownerId=input.ownerId; s.keyId=input.keyId; s.amount=input.amount; s.policy=input.policy; s.valid=true; return true; }
const SaveTransactionCoordinatorSnapshot* SaveTransactionCoordinatorModel::get(std::uint64_t keyId) const { auto it=data_.find(keyId); return it==data_.end()?nullptr:&it->second; }
std::vector<SaveTransactionCoordinatorSnapshot> SaveTransactionCoordinatorModel::ordered() const { std::vector<SaveTransactionCoordinatorSnapshot> out; out.reserve(data_.size()); for(const auto& [id,s]:data_) if(s.valid) out.push_back(s); std::sort(out.begin(),out.end(),[](const auto&a,const auto&b){return a.keyId<b.keyId;}); return out; }
void SaveTransactionCoordinatorModel::clear() { data_.clear(); revision_=1; }
}
