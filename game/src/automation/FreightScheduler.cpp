// Intended function: Schedule conveyors, loaders, drones, cargo rail, and ship transfers using stable endpoints and bounded queues.
#include "FreightScheduler.hpp"
namespace elysium::automation {
std::uint64_t FreightOrderRegistry::key(const FreightOrder& r) noexcept { return static_cast<std::uint64_t>(r.orderId); }
bool FreightOrderRegistry::publish(FreightOrder r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const FreightOrder& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool FreightOrderRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const FreightOrder& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const FreightOrder* FreightOrderRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const FreightOrder& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::automation
