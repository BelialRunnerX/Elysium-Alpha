// Intended function: score biomes without face-local tables or resource-abundance overrides.
#include "world/BiomeDecisionModel.hpp"

#include "core/Saturating.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace elysium {

BiomeDecision BiomeDecisionModel::select(
    PlanetClass planetClass,
    const PlanetFieldSample& raw,
    double elevation01,
    std::span<const BiomeDecisionDefinition> definitions) const noexcept {
    const auto classBit = std::uint32_t{1} << static_cast<std::uint32_t>(planetClass);
    const double temp = safe::finiteClamp(raw.temperature01, 0.0, 1.0);
    const double moisture = safe::finiteClamp(raw.moisture01, 0.0, 1.0);
    elevation01 = safe::finiteClamp(elevation01, 0.0, 1.0);

    const BiomeDecisionDefinition* best = nullptr;
    double bestScore = std::numeric_limits<double>::infinity();
    for (const auto& d : definitions) {
        if (d.biomeId == 0 || (d.allowedPlanetClassMask & classBit) == 0) continue;
        const double tw = std::max(0.001, safe::nonNegative(d.temperatureWidth, 1.0));
        const double mw = std::max(0.001, safe::nonNegative(d.moistureWidth, 1.0));
        const double ew = std::max(0.001, safe::nonNegative(d.elevationWidth, 1.0));
        const double dt = (temp - safe::finiteClamp(d.targetTemperature01,0.0,1.0))/tw;
        const double dm = (moisture - safe::finiteClamp(d.targetMoisture01,0.0,1.0))/mw;
        const double de = (elevation01 - safe::finiteClamp(d.targetElevation01,0.0,1.0))/ew;
        const double score = safe::nonNegative(dt*dt + dm*dm + de*de);
        if (score < bestScore || (score == bestScore && best && d.biomeId < best->biomeId)) {
            best = &d; bestScore = score;
        }
    }
    if (!best) return {};
    return {
        .biomeId = best->biomeId,
        .score = safe::nonNegative(bestScore),
        .floraDensity = safe::nonNegative(best->floraDensity),
        .faunaDensity = safe::nonNegative(best->faunaDensity),
        .structuralWeathering = safe::finiteClamp(best->structuralWeathering,0.0,1.0),
        .surfaceMaterialKey = best->surfaceMaterialKey,
        .subsurfaceMaterialKey = best->subsurfaceMaterialKey,
        .detailProfileKey = best->detailProfileKey,
    };
}

} // namespace elysium
