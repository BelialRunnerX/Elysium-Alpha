#include "history/PersonalLegacy.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Summarize notable individual deeds and relationships for retirement, death, succession, statues, naming, and Chronicle use.
bool PersonalLegacyModel::update(const PersonalLegacyInput& input) { if(input.keyId==0) return false; auto& s=data_[input.keyId]; s.revision=revision_++; s.ownerId=input.ownerId; s.keyId=input.keyId; s.amount=input.amount; s.policy=input.policy; s.valid=true; return true; }
const PersonalLegacySnapshot* PersonalLegacyModel::get(std::uint64_t keyId) const { auto it=data_.find(keyId); return it==data_.end()?nullptr:&it->second; }
std::vector<PersonalLegacySnapshot> PersonalLegacyModel::ordered() const { std::vector<PersonalLegacySnapshot> out; out.reserve(data_.size()); for(const auto& [id,s]:data_) if(s.valid) out.push_back(s); std::sort(out.begin(),out.end(),[](const auto&a,const auto&b){return a.keyId<b.keyId;}); return out; }
void PersonalLegacyModel::clear() { data_.clear(); revision_=1; }
}
