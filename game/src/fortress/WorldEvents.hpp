#pragma once

#include "fortress/Components.hpp"

#include <string>

namespace elysium::fortress {

enum class PlanetaryEventKind : std::uint8_t {
    StormFront,
    MeteorFall,
    SporeBloom,
    Migration,
    CaveCollapse,
    AquiferBreach,
    VolcanicSurge,
    RadiationPulse,
    UnswornCaravan,
    ImperialSurvey,
    DistressCall,
    CelestialAlignment
};

struct PlanetaryEvent {
    StableId id{};
    PlanetaryEventKind kind{PlanetaryEventKind::StormFront};
    SiteId site{};
    SpatialAnchor location{};
    float intensity{};
    float durationHours{};
    float progress{};
    bool signaled{};
    bool resolved{};
    std::string playerDecisionHint;
};

PlanetaryEvent generatePlanetaryEvent(std::uint64_t seed, SiteId site, std::uint64_t epoch,
                                      PlanetaryEventKind kind);
void advancePlanetaryEvent(PlanetaryEvent& event, float hours, float mitigation);

} // namespace elysium::fortress
