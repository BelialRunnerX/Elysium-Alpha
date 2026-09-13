// Intended function: Represent future multiplayer interest regions, stable replicated objects, priority, relevancy, and bandwidth estimates.
#include "ReplicationInterest.hpp"
namespace elysium::network {
std::uint64_t ReplicationInterestRecordRegistry::key(const ReplicationInterestRecord& r) noexcept { return static_cast<std::uint64_t>(r.observerId); }
bool ReplicationInterestRecordRegistry::publish(ReplicationInterestRecord r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ReplicationInterestRecord& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool ReplicationInterestRecordRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ReplicationInterestRecord& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const ReplicationInterestRecord* ReplicationInterestRecordRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ReplicationInterestRecord& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::network
