// Intended function: deterministic decision-facing biome selection from continuous spherical fields.
#pragma once

#include "world/PlanetFieldGenerator.hpp"
#include "world/PlanetTypes.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace elysium {

struct BiomeDecisionDefinition {
    std::uint64_t biomeId{};
    std::uint32_t allowedPlanetClassMask{0xFFFFFFFFu};
    double targetTemperature01{0.5};
    double targetMoisture01{0.5};
    double targetElevation01{0.5};
    double temperatureWidth{0.5};
    double moistureWidth{0.5};
    double elevationWidth{0.5};
    double floraDensity{};
    double faunaDensity{};
    double structuralWeathering{};
    std::uint64_t surfaceMaterialKey{};
    std::uint64_t subsurfaceMaterialKey{};
    std::uint64_t detailProfileKey{};
};

struct BiomeDecision {
    std::uint64_t biomeId{};
    double score{};
    double floraDensity{};
    double faunaDensity{};
    double structuralWeathering{};
    std::uint64_t surfaceMaterialKey{};
    std::uint64_t subsurfaceMaterialKey{};
    std::uint64_t detailProfileKey{};
};

class BiomeDecisionModel final {
public:
    [[nodiscard]] BiomeDecision select(
        PlanetClass planetClass,
        const PlanetFieldSample& field,
        double elevation01,
        std::span<const BiomeDecisionDefinition> definitions) const noexcept;
};

} // namespace elysium
