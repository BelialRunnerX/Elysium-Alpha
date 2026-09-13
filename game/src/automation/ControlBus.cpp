// Intended function: Route bounded stable-address control messages between machines, sensors, doors, logistics, and defenses.
#include "ControlBus.hpp"
namespace elysium::automation {
std::uint64_t ControlMessageRegistry::key(const ControlMessage& r) noexcept { return static_cast<std::uint64_t>(r.messageId); }
bool ControlMessageRegistry::publish(ControlMessage r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ControlMessage& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool ControlMessageRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ControlMessage& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const ControlMessage* ControlMessageRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ControlMessage& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::automation
