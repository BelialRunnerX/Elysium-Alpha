// Intended function: Predict bounded near-term weather transitions from authoritative climate/weather state for planning and UI.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::climate {
struct WeatherForecastState {
    std::uint64_t forecastId{};
    std::uint64_t weatherId{};
    std::uint64_t startTick{};
    std::uint64_t durationTicks{};
    double confidence{};
    double hazard{};
};
class WeatherForecastStateStore {
public:
 bool put(WeatherForecastState v); bool erase(std::uint64_t id);
 [[nodiscard]] const WeatherForecastState* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<WeatherForecastState>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const WeatherForecastState& v) noexcept; std::vector<WeatherForecastState> values_;
};
} // namespace elysium::climate
