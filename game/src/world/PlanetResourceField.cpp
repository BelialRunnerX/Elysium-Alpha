// Intended function: independent local ore variation layered over universe-wide per-material abundance.
#include "world/PlanetResourceField.hpp"

#include "core/Determinism.hpp"
#include "core/Saturating.hpp"

#include <algorithm>
#include <cmath>

namespace elysium {
namespace {
double unit(std::uint64_t x) noexcept {
    return static_cast<double>((mix64(x) >> 11U) & ((1ULL << 53U) - 1ULL)) /
           static_cast<double>(1ULL << 53U);
}

std::uint64_t quantized(double value, double scale) noexcept {
    const double v = safe::finiteClamp(value, 0.0, 1.0);
    return static_cast<std::uint64_t>(std::floor(v * scale));
}
} // namespace

double PlanetResourceField::localNoise(
    std::uint64_t materialKey,
    std::uint64_t planetSeed,
    const PlanetResourcePoint& point) const noexcept {
    // Multi-scale hashed cells: cheap, deterministic and independent by material.
    // This is intentionally local/sparse; no dense world-sized ore array exists.
    const auto base = fields_->subSeed(0x504C4E545F4F5245ULL, materialKey, planetSeed); // PLNT_ORE
    double sum = 0.0;
    double weight = 0.0;
    for (std::uint64_t octave = 0; octave < 5; ++octave) {
        const double scale = static_cast<double>(1ULL << (octave + 3ULL));
        const auto x = quantized(point.x01, scale);
        const auto y = quantized(point.y01, scale);
        const auto z = quantized(point.z01, scale);
        const auto d = quantized(point.depth01, scale);
        auto h = mix64(base ^ mix64(octave));
        h = mix64(h ^ mix64(x));
        h = mix64(h ^ mix64(y << 1U));
        h = mix64(h ^ mix64(z << 2U));
        h = mix64(h ^ mix64(d << 3U));
        const double w = 1.0 / static_cast<double>(1ULL << octave);
        sum += unit(h) * w;
        weight += w;
    }
    return safe::finiteClamp(weight > 0.0 ? sum / weight : 0.0, 0.0, 1.0);
}

PlanetResourceSample PlanetResourceField::sample(
    std::uint64_t materialKey,
    const UniversePoint& systemPoint,
    std::uint64_t planetSeed,
    const PlanetResourcePoint& point) const {
    const SpaceResourceContext planetOnly{};
    const auto global = fields_->sampleResource(materialKey, systemPoint, planetSeed, planetOnly);
    const double local = localNoise(materialKey, planetSeed, point);
    const auto localSeed = fields_->subSeed(0x504C4E545F475244ULL, materialKey, planetSeed); // PLNT_GRD
    const double depthPreference = unit(localSeed ^ 0x91ULL);
    const double depthWidth = 0.08 + unit(localSeed ^ 0x92ULL) * 0.72;
    const double depthAffinity = std::exp(-std::abs(safe::finiteClamp(point.depth01, 0.0, 1.0) - depthPreference) /
                                          std::max(0.01, depthWidth));
    const double abundance = safe::nonNegative(global.planetaryAbundance);
    const double localAbundance = safe::nonNegative(abundance * (0.20 + local * 1.80) * (0.35 + depthAffinity * 0.95));
    const double veinProbability = safe::finiteClamp(
        localAbundance / (1.0 + localAbundance) * (0.25 + local * 0.75), 0.0, 1.0);
    const double grade = safe::nonNegative(localAbundance * (0.45 + unit(localSeed ^ quantized(local, 65535.0)) * 1.55));
    return {
        .materialKey = materialKey,
        .globalAbundance = abundance,
        .localAbundance = localAbundance,
        .veinProbability = veinProbability,
        .grade = grade,
    };
}

} // namespace elysium
