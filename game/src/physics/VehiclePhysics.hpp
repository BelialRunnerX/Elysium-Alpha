// Intended function: Represent simplified vehicle traction, steering, acceleration, suspension, slope, water/hover, and collision response state.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::physics {
struct VehiclePhysicsState {
    std::uint64_t vehicleId{};
    double speed{};
    double steering{};
    double traction{};
    double slope{};
    std::uint64_t mode{};
};
class VehiclePhysicsStateIndex {
public:
 bool upsert(VehiclePhysicsState value); bool erase(std::uint64_t id); [[nodiscard]] const VehiclePhysicsState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<VehiclePhysicsState>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const VehiclePhysicsState& value) noexcept; std::vector<VehiclePhysicsState> rows_;
};
}
