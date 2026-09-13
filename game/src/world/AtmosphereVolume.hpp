// Intended function: bounded habitat/local-gas atmosphere state and deterministic transfer contracts.
#pragma once

#include "core/Saturating.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace elysium {

struct VolumeTopologyResult {
    std::uint64_t volumeId{};
    std::uint64_t visitedCells{};
    bool reachedSky{};
    bool budgetExhausted{};
    bool terminatedBounded{};

    [[nodiscard]] bool sealed() const noexcept {
        return terminatedBounded && !reachedSky && !budgetExhausted;
    }
};

struct AtmosphereState {
    std::uint64_t volumeId{};
    double pressureKPa{};
    double oxygenFraction{};
    double toxicFraction{};
    double smokeFraction{};
    double temperatureK{};
    double gasAmount{};
    bool sealed{};
};

struct AtmosphereExchange {
    std::uint64_t fromVolume{};
    std::uint64_t toVolume{};
    double conductance{};
};

struct AtmosphereDelta {
    std::uint64_t volumeId{};
    double pressureDeltaKPa{};
    double oxygenDelta{};
    double toxicDelta{};
    double smokeDelta{};
    double gasDelta{};
};

class AtmosphereSolver final {
public:
    [[nodiscard]] std::vector<AtmosphereDelta> compute(
        std::span<const AtmosphereState> volumes,
        std::span<const AtmosphereExchange> exchanges,
        double stepSeconds) const;
};

[[nodiscard]] AtmosphereState sanitizeAtmosphere(AtmosphereState value) noexcept;

} // namespace elysium
