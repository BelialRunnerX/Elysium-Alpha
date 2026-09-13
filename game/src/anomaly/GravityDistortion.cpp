#include "anomaly/GravityDistortion.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Represent anomaly gravity wells, directional changes, movement hazards, and physics query modifiers.
bool GravityDistortionModel::update(const GravityDistortionInput& input) { if(input.keyId==0) return false; auto& s=data_[input.keyId]; s.revision=revision_++; s.ownerId=input.ownerId; s.keyId=input.keyId; s.amount=input.amount; s.policy=input.policy; s.valid=true; return true; }
const GravityDistortionSnapshot* GravityDistortionModel::get(std::uint64_t keyId) const { auto it=data_.find(keyId); return it==data_.end()?nullptr:&it->second; }
std::vector<GravityDistortionSnapshot> GravityDistortionModel::ordered() const { std::vector<GravityDistortionSnapshot> out; out.reserve(data_.size()); for(const auto& [id,s]:data_) if(s.valid) out.push_back(s); std::sort(out.begin(),out.end(),[](const auto&a,const auto&b){return a.keyId<b.keyId;}); return out; }
void GravityDistortionModel::clear() { data_.clear(); revision_=1; }
}
