// Intended function: Represent deterministic automation nodes/edges, finite rules, stable ports, priorities, and evaluation revisions.
#include "LogicGraph.hpp"
namespace elysium::automation {
std::uint64_t LogicNodeStateRegistry::key(const LogicNodeState& r) noexcept { return static_cast<std::uint64_t>(r.nodeId); }
bool LogicNodeStateRegistry::publish(LogicNodeState r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const LogicNodeState& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool LogicNodeStateRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const LogicNodeState& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const LogicNodeState* LogicNodeStateRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const LogicNodeState& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::automation
