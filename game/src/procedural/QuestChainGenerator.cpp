#include "procedural/QuestChainGenerator.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Generate linked multi-stage quests with alternative branches, faction choices, world-state gates, and Chronicle significance.
bool QuestChainGeneratorStore::apply(const QuestChainGeneratorOp& o) { if(!o.id) return false; auto& d=data_[o.id]; d.revision=revision_++; d.id=o.id; d.owner=o.owner; d.ref=o.ref; d.value=o.value; d.flags=o.flags; d.live=true; return true; }
bool QuestChainGeneratorStore::erase(std::uint64_t id) { return data_.erase(id)!=0; }
const QuestChainGeneratorData* QuestChainGeneratorStore::find(std::uint64_t id) const { auto it=data_.find(id); return it==data_.end()?nullptr:&it->second; }
std::vector<QuestChainGeneratorData> QuestChainGeneratorStore::snapshot() const { std::vector<QuestChainGeneratorData> v; v.reserve(data_.size()); for(auto& [id,d]:data_) if(d.live) v.push_back(d); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.id<b.id;}); return v; }
void QuestChainGeneratorStore::clear() { data_.clear(); revision_=1; }
}
