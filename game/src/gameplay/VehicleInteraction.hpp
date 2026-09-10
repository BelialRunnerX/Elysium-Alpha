// Intended function: Manage enter/exit/seat/control ownership, cargo access, repair/refuel, autopilot handoff, and safe dismount requests.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::gameplay {
struct VehicleInteractionState {
    std::uint64_t vehicleId{};
    std::uint64_t actorId{};
    std::uint64_t seatId{};
    std::uint64_t controlMode{};
    std::uint64_t cargoAccess{};
    std::uint64_t flags{};
};
class VehicleInteractionStateStore {
public:
 bool put(VehicleInteractionState v); bool erase(std::uint64_t id);
 [[nodiscard]] const VehicleInteractionState* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<VehicleInteractionState>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const VehicleInteractionState& v) noexcept; std::vector<VehicleInteractionState> values_;
};
} // namespace elysium::gameplay
