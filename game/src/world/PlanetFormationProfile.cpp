// Intended function: deterministic planet-class/environment projection from parent star and lazy orbit candidate.
#include "world/PlanetFormationProfile.hpp"

#include "core/Determinism.hpp"
#include "core/Saturating.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace elysium {
namespace {
double unit(std::uint64_t x) noexcept {
    return static_cast<double>((mix64(x) >> 11U) & ((1ULL << 53U) - 1ULL)) /
           static_cast<double>(1ULL << 53U);
}
} // namespace

PlanetPhysicalProfile PlanetFormationProfiler::describe(
    GalaxySystemId system,
    PlanetCandidateId candidate) const {
    const auto sys = galaxy_->describe(system);
    const auto planet = galaxy_->planetCandidate(system, candidate);
    const auto seed = planet.stableSeed;
    const double distance = std::max(1.0e-6, safe::nonNegative(planet.formation.orbitalDistanceAu));
    const double irradiation = safe::nonNegative(sys.starFormationSignal / (distance * distance + 0.05));

    // All environmental characteristics are seeded; fixed coefficients are only
    // algorithm transforms. Orbit/star context biases but does not dictate class.
    const double coldBias = safe::finiteClamp(distance / (1.0 + distance), 0.0, 1.0);
    const double heatBias = safe::finiteClamp(irradiation / (1.0 + irradiation), 0.0, 1.0);
    const double volatiles = safe::finiteClamp(0.15 + unit(seed ^ 0x11ULL) * 0.85 + coldBias * 0.25, 0.0, 1.0);
    const double ocean = safe::finiteClamp(volatiles * (0.35 + unit(seed ^ 0x22ULL) * 0.85), 0.0, 1.0);
    const double toxicity = safe::finiteClamp(unit(seed ^ 0x33ULL) * (0.65 + sys.metallicity01 * 0.45), 0.0, 1.0);
    const double radiation = safe::finiteClamp(unit(seed ^ 0x44ULL) * 0.65 + heatBias * 0.45, 0.0, 1.0);
    const double anomaly = safe::finiteClamp(unit(seed ^ 0x55ULL) * unit(seed ^ 0x56ULL) * 1.35, 0.0, 1.0);

    struct Score { PlanetClass type; double value; };
    const std::array<Score, 8> scores{{
        {PlanetClass::Temperate, safe::finiteClamp((1.0 - std::abs(heatBias - 0.45)) * (0.55 + ocean * 0.45), 0.0, 1.5)},
        {PlanetClass::Barren, safe::finiteClamp((1.0 - ocean) * (0.45 + unit(seed ^ 0x61ULL) * 0.55), 0.0, 1.5)},
        {PlanetClass::Scorched, safe::finiteClamp(heatBias * (0.65 + unit(seed ^ 0x62ULL) * 0.55), 0.0, 1.5)},
        {PlanetClass::Frozen, safe::finiteClamp(coldBias * (0.65 + volatiles * 0.45), 0.0, 1.5)},
        {PlanetClass::Toxic, safe::finiteClamp(toxicity * (0.70 + volatiles * 0.35), 0.0, 1.5)},
        {PlanetClass::Irradiated, safe::finiteClamp(radiation * (0.65 + sys.metallicity01 * 0.45), 0.0, 1.5)},
        {PlanetClass::Oceanic, safe::finiteClamp(ocean * (0.75 + unit(seed ^ 0x67ULL) * 0.35), 0.0, 1.5)},
        {PlanetClass::Anomalous, safe::finiteClamp(anomaly * (0.75 + unit(seed ^ 0x68ULL) * 0.65), 0.0, 1.5)},
    }};
    auto best = scores.front();
    for (const auto& score : scores) if (score.value > best.value) best = score;

    return {
        .systemId = system,
        .candidateId = candidate,
        .stableSeed = seed,
        .planetClass = best.type,
        .orbitalDistanceAu = safe::nonNegative(planet.formation.orbitalDistanceAu),
        .radiusKm = safe::nonNegative(planet.formation.radiusKm),
        .irradiation = irradiation,
        .volatilePotential = volatiles,
        .oceanPotential = ocean,
        .toxicityPotential = toxicity,
        .radiationPotential = radiation,
        .anomalyPotential = anomaly,
    };
}

} // namespace elysium
