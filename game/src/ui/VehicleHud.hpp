// Intended function: Project vehicle condition, energy/fuel, cargo, seats, terrain/navigation, modules, hazards, and active control mode.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ui {
struct VehicleHudState {
    std::uint64_t vehicleId{};
    double condition{};
    double energy{};
    double cargo{};
    double speed{};
    std::uint64_t controlMode{};
};
class VehicleHudStateTable {
public:
 bool set(VehicleHudState value); bool remove(std::uint64_t id);
 [[nodiscard]] const VehicleHudState* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<VehicleHudState> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const VehicleHudState& value) noexcept; std::vector<VehicleHudState> rows_;
};
}
