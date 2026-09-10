// Intended function: Coordinate multi-ship fleet movement, formation cohesion, fuel budgets, route legs, and arrival synchronization.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::travel {
struct FleetTravelState {
    std::uint64_t fleetId{};
    std::uint64_t leaderShipId{};
    std::uint64_t routeId{};
    double fuelBudget{};
    double cohesion{};
    std::uint64_t etaTicks{};
};
class FleetTravelStateRegistry {
public:
    bool publish(FleetTravelState record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const FleetTravelState* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<FleetTravelState>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const FleetTravelState& r) noexcept;
    std::vector<FleetTravelState> records_;
};
} // namespace elysium::travel
