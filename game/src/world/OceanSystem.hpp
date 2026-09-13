// Intended function: Represent ocean/lake water bodies, sea level, currents, salinity, contamination, freezing, and resource zones.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::world {
struct WaterBodyState {
    double waterBodyId{};
    double seaLevel{};
    double salinity{};
    double currentStrength{};
    double temperature{};
    double contamination{};
};
class WaterBodyStateStore {
public:
 bool put(WaterBodyState v); bool erase(std::uint64_t id);
 [[nodiscard]] const WaterBodyState* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<WaterBodyState>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const WaterBodyState& v) noexcept; std::vector<WaterBodyState> values_;
};
} // namespace elysium::world
