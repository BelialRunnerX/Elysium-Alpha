// Intended function: Represent compact deterministic behavior-tree runtime nodes, cursors, cooldowns, and stable blackboard keys.
#include "BehaviorTree.hpp"
namespace elysium::ai {
std::uint64_t BehaviorNodeStateTable::keyOf(const BehaviorNodeState& v) noexcept { return static_cast<std::uint64_t>(v.actorId); }
bool BehaviorNodeStateTable::set(BehaviorNodeState v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const BehaviorNodeState& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool BehaviorNodeStateTable::remove(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const BehaviorNodeState& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const BehaviorNodeState* BehaviorNodeStateTable::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const BehaviorNodeState& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
