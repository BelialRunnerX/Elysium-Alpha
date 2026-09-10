// Intended function: Represent exclusive/shared inventory reservations with owner/dependent stable IDs, leases, quantity, priority, and invalidation state.
#include "ReservationSystem.hpp"
namespace elysium::inventory {
std::uint64_t InventoryReservationIndex::keyOf(const InventoryReservation& v) noexcept { return static_cast<std::uint64_t>(v.reservationId); }
bool InventoryReservationIndex::upsert(InventoryReservation v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const InventoryReservation& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool InventoryReservationIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const InventoryReservation& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const InventoryReservation* InventoryReservationIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const InventoryReservation& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
