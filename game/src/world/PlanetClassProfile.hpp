// Intended function: decision-facing planet hazard/build identity without overriding seeded material abundance.
#pragma once

#include "world/PlanetFormationProfile.hpp"

#include <cstdint>

namespace elysium {

enum class PrimaryHazard : std::uint8_t { None, Vacuum, Thermal, Cryogenic, Corrosive, Radiological, Pressure, Mixed };

struct PlanetClassDecisionProfile {
    PlanetClass planetClass{PlanetClass::Temperate};
    PrimaryHazard primaryHazard{PrimaryHazard::None};
    double shelterPressure{};
    double thermalEngineeringPressure{};
    double corrosionEngineeringPressure{};
    double radiationEngineeringPressure{};
    double pressureEngineeringPressure{};
    bool naturallyBreathableCandidate{};
    bool claimable{true};
};

[[nodiscard]] PlanetClassDecisionProfile decisionProfile(const PlanetPhysicalProfile& planet) noexcept;

} // namespace elysium
