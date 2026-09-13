// Intended function: Manage orbital station modules, docking, power, atmosphere, cargo, population, defenses, and expansion anchors.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::orbital {
struct StationState {
    std::uint64_t stationId{};
    std::uint64_t systemId{};
    std::uint64_t moduleCount{};
    double powerBalance{};
    double population{};
    double readiness{};
};
class StationStateStore {
public:
 bool put(StationState v); bool erase(std::uint64_t id);
 [[nodiscard]] const StationState* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<StationState>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const StationState& v) noexcept; std::vector<StationState> values_;
};
} // namespace elysium::orbital
