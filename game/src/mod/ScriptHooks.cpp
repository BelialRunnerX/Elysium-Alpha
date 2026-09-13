// Intended function: Expose bounded event-hook declarations for future scripting while keeping world mutation behind validated command APIs.
#include "ScriptHooks.hpp"
namespace elysium::mod {
std::uint64_t ScriptHookRecordRegistry::key(const ScriptHookRecord& r) noexcept { return static_cast<std::uint64_t>(r.hookId); }
bool ScriptHookRecordRegistry::publish(ScriptHookRecord r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ScriptHookRecord& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool ScriptHookRecordRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ScriptHookRecord& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const ScriptHookRecord* ScriptHookRecordRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ScriptHookRecord& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::mod
