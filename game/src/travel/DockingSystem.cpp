// Intended function: Track docking ports, reservations, approach queues, compatibility, pressurization, and transfer readiness.
#include "DockingSystem.hpp"
namespace elysium::travel {
std::uint64_t DockingReservationRegistry::key(const DockingReservation& r) noexcept { return static_cast<std::uint64_t>(r.reservationId); }
bool DockingReservationRegistry::publish(DockingReservation r) {
    auto id=key(r); if(id==0) return false;
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const DockingReservation& a,std::uint64_t b){return key(a)<b;});
    if(it!=records_.end()&&key(*it)==id){*it=r;return true;} records_.insert(it,r); return true;
}
bool DockingReservationRegistry::remove(std::uint64_t id) {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const DockingReservation& a,std::uint64_t b){return key(a)<b;});
    if(it==records_.end()||key(*it)!=id)return false; records_.erase(it); return true;
}
const DockingReservation* DockingReservationRegistry::lookup(std::uint64_t id) const {
    auto it=std::lower_bound(records_.begin(),records_.end(),id,[](const DockingReservation& a,std::uint64_t b){return key(a)<b;});
    return it!=records_.end()&&key(*it)==id?&*it:nullptr;
}
} // namespace elysium::travel
