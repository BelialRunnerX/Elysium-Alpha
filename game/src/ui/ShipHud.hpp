// Intended function: Project ship hull, shields, power, heat, fuel, cargo, modules, navigation, docking, hazard, and warp readiness.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ui {
struct ShipHudState {
    std::uint64_t shipId{};
    double hull{};
    double shield{};
    double power{};
    double fuel{};
    double warpCharge{};
};
class ShipHudStateTable {
public:
 bool set(ShipHudState value); bool remove(std::uint64_t id);
 [[nodiscard]] const ShipHudState* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<ShipHudState> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const ShipHudState& value) noexcept; std::vector<ShipHudState> rows_;
};
}
