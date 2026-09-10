// Intended function: Provide stable ship hull definitions, module capacities, cargo limits, mass, thrust class, and progression tier.
#include "ShipCatalogue.hpp"
namespace elysium::content {
std::uint64_t ShipRecordRegistry::key(const ShipRecord& r) noexcept { return static_cast<std::uint64_t>(r.shipId); }
bool ShipRecordRegistry::publish(ShipRecord r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ShipRecord& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool ShipRecordRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ShipRecord& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const ShipRecord* ShipRecordRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const ShipRecord& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::content
