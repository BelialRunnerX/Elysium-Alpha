// Intended function: Represent simplified deterministic atmospheric/orbital flight state for thrust, velocity, lift proxy, drag, and autopilot.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::travel {
struct FlightState {
    std::uint64_t vehicleId{};
    double speed{};
    double altitude{};
    double thrust{};
    double drag{};
    std::uint64_t autopilotMode{};
};
class FlightStateRegistry {
public:
    bool publish(FlightState record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const FlightState* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<FlightState>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const FlightState& r) noexcept;
    std::vector<FlightState> records_;
};
} // namespace elysium::travel
