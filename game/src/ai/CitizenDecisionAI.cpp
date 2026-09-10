#include "ai/CitizenDecisionAI.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Select bounded citizen goals from needs, jobs, schedules, relationships, emergencies, and personal priorities.
bool CitizenDecisionAIStore::apply(const CitizenDecisionAIOp& o) { if(!o.id) return false; auto& d=data_[o.id]; d.revision=revision_++; d.id=o.id; d.owner=o.owner; d.ref=o.ref; d.value=o.value; d.flags=o.flags; d.live=true; return true; }
bool CitizenDecisionAIStore::erase(std::uint64_t id) { return data_.erase(id)!=0; }
const CitizenDecisionAIData* CitizenDecisionAIStore::find(std::uint64_t id) const { auto it=data_.find(id); return it==data_.end()?nullptr:&it->second; }
std::vector<CitizenDecisionAIData> CitizenDecisionAIStore::snapshot() const { std::vector<CitizenDecisionAIData> v; v.reserve(data_.size()); for(auto& [id,d]:data_) if(d.live) v.push_back(d); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.id<b.id;}); return v; }
void CitizenDecisionAIStore::clear() { data_.clear(); revision_=1; }
}
