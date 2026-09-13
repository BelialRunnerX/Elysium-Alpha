// Intended function: Track docking ports, reservations, approach queues, compatibility, pressurization, and transfer readiness.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::travel {
struct DockingReservation {
    std::uint64_t reservationId{};
    std::uint64_t portId{};
    std::uint64_t shipId{};
    std::uint64_t priority{};
    std::uint64_t expiresTick{};
    std::uint64_t state{};
};
class DockingReservationRegistry {
public:
    bool publish(DockingReservation record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const DockingReservation* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<DockingReservation>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const DockingReservation& r) noexcept;
    std::vector<DockingReservation> records_;
};
} // namespace elysium::travel
