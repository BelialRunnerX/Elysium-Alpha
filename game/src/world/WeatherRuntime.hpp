// Intended function: deterministic weather-state transitions driven by seed, local fields, and time buckets.
#pragma once

#include "world/PlanetFieldGenerator.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace elysium {

struct WeatherStateDefinition {
    std::uint64_t weatherId{};
    double baseOccurrence{};
    double targetTemperature01{0.5};
    double targetMoisture01{0.5};
    double targetRadiation01{};
    double hazardMultiplier{1.0};
    double visibilityMultiplier{1.0};
    double precipitation01{};
    double windMultiplier{1.0};
    std::uint64_t presentationTag{};
};

struct WeatherStateSample {
    std::uint64_t weatherId{};
    std::uint64_t timeBucket{};
    double hazardMultiplier{1.0};
    double visibilityMultiplier{1.0};
    double precipitation01{};
    double windMultiplier{1.0};
    std::uint64_t presentationTag{};
};

class WeatherRuntime final {
public:
    explicit WeatherRuntime(std::uint64_t planetSeed) : seed_(planetSeed) {}
    [[nodiscard]] WeatherStateSample sample(
        std::uint64_t timeBucket,
        const PlanetFieldSample& field,
        std::span<const WeatherStateDefinition> definitions) const noexcept;
private:
    std::uint64_t seed_{};
};

} // namespace elysium
