// Intended function: Translate authoritative weather/climate hazards into player/citizen exposure inputs and preparation/readiness diagnostics.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::integration {
struct WeatherExposureIntent {
    std::uint64_t intentId{};
    std::uint64_t actorId{};
    std::uint64_t weatherId{};
    double temperature{};
    double radiation{};
    double severity{};
};
class WeatherExposureIntentIndex {
public:
 bool upsert(WeatherExposureIntent value); bool erase(std::uint64_t id); [[nodiscard]] const WeatherExposureIntent* find(std::uint64_t id) const; [[nodiscard]] const std::vector<WeatherExposureIntent>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const WeatherExposureIntent& value) noexcept; std::vector<WeatherExposureIntent> rows_;
};
}
