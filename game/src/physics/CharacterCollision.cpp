#include "physics/CharacterCollision.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Resolve capsule/shape movement queries against voxel and object collision surfaces for direct-operative control.
bool CharacterCollisionModel::update(const CharacterCollisionInput& input) { if(input.keyId==0) return false; auto& s=data_[input.keyId]; s.revision=revision_++; s.ownerId=input.ownerId; s.keyId=input.keyId; s.amount=input.amount; s.policy=input.policy; s.valid=true; return true; }
const CharacterCollisionSnapshot* CharacterCollisionModel::get(std::uint64_t keyId) const { auto it=data_.find(keyId); return it==data_.end()?nullptr:&it->second; }
std::vector<CharacterCollisionSnapshot> CharacterCollisionModel::ordered() const { std::vector<CharacterCollisionSnapshot> out; out.reserve(data_.size()); for(const auto& [id,s]:data_) if(s.valid) out.push_back(s); std::sort(out.begin(),out.end(),[](const auto&a,const auto&b){return a.keyId<b.keyId;}); return out; }
void CharacterCollisionModel::clear() { data_.clear(); revision_=1; }
}
