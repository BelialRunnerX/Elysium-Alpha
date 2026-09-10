// Intended function: advance objective state monotonically and unlock dependent objectives only after durable prerequisite completion.
#include "gameplay/QuestObjectives.hpp"
#include <algorithm>
namespace elysium{
bool ObjectiveGraph::add(ObjectiveRecord r){if(!r.objectiveId||records_.contains(r.objectiveId))return false;records_.emplace(r.objectiveId,std::move(r));refreshUnlocks();return true;}
bool ObjectiveGraph::activate(std::uint64_t id){auto i=records_.find(id);if(i==records_.end()||i->second.state!=ObjectiveState::Locked)return false;for(auto p:i->second.prerequisites){auto x=records_.find(p);if(x==records_.end()||x->second.state!=ObjectiveState::Complete)return false;}i->second.state=ObjectiveState::Active;return true;}
bool ObjectiveGraph::addProgress(std::uint64_t id,std::uint32_t n){auto i=records_.find(id);if(i==records_.end()||i->second.state!=ObjectiveState::Active)return false;i->second.progress=std::min(i->second.required,i->second.progress+n);if(i->second.progress>=i->second.required){i->second.state=ObjectiveState::Complete;refreshUnlocks();}return true;}
bool ObjectiveGraph::fail(std::uint64_t id){auto i=records_.find(id);if(i==records_.end()||i->second.state==ObjectiveState::Complete)return false;i->second.state=ObjectiveState::Failed;return true;}
void ObjectiveGraph::refreshUnlocks(){std::vector<std::uint64_t>ids;for(auto&[id,_]:records_)ids.push_back(id);std::sort(ids.begin(),ids.end());for(auto id:ids){auto&r=records_[id];if(r.state!=ObjectiveState::Locked)continue;bool ok=true;for(auto p:r.prerequisites){auto x=records_.find(p);if(x==records_.end()||x->second.state!=ObjectiveState::Complete){ok=false;break;}}if(ok)r.state=ObjectiveState::Active;}}
std::optional<ObjectiveRecord>ObjectiveGraph::find(std::uint64_t id)const{auto i=records_.find(id);return i==records_.end()?std::nullopt:std::optional<ObjectiveRecord>(i->second);}std::vector<ObjectiveRecord>ObjectiveGraph::visible()const{std::vector<ObjectiveRecord>o;for(auto&[_,r]:records_)if(r.state!=ObjectiveState::Locked)o.push_back(r);std::sort(o.begin(),o.end(),[](auto&a,auto&b){return a.objectiveId<b.objectiveId;});return o;}
}
