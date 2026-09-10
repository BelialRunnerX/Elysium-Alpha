// Intended function: Represent exclusive/shared inventory reservations with owner/dependent stable IDs, leases, quantity, priority, and invalidation state.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::inventory {
struct InventoryReservation {
    std::uint64_t reservationId{};
    std::uint64_t assetId{};
    std::uint64_t ownerId{};
    std::uint64_t dependentId{};
    std::uint64_t units{};
    std::uint64_t expiresTick{};
};
class InventoryReservationIndex {
public:
 bool upsert(InventoryReservation value); bool erase(std::uint64_t id); [[nodiscard]] const InventoryReservation* find(std::uint64_t id) const; [[nodiscard]] const std::vector<InventoryReservation>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const InventoryReservation& value) noexcept; std::vector<InventoryReservation> rows_;
};
}
