// Intended function: Provide stable dialogue nodes, speaker roles, conditions, choices, consequences, and localization keys.
#include "DialogueCatalogue.hpp"
namespace elysium::content {
std::uint64_t DialogueNodeRegistry::key(const DialogueNode& r) noexcept { return static_cast<std::uint64_t>(r.nodeId); }
bool DialogueNodeRegistry::publish(DialogueNode r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const DialogueNode& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool DialogueNodeRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const DialogueNode& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const DialogueNode* DialogueNodeRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const DialogueNode& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::content
