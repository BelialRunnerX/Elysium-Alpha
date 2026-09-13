// Intended function: deterministic sparse universe fields for stars, planets and independent material abundance.
#include "galaxy/UniverseFormationFields.hpp"

#include "core/Determinism.hpp"
#include "core/Saturating.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>

namespace elysium {
namespace {

constexpr std::uint64_t DomainStar = 0x535441525F464945ULL;      // STAR_FIE
constexpr std::uint64_t DomainPlanet = 0x504C414E45545F46ULL;    // PLANET_F
constexpr std::uint64_t DomainResource = 0x5245534F55524345ULL;  // RESOURCE
constexpr std::uint64_t DomainLight = 0x4C494748545F5052ULL;     // LIGHT_PR
constexpr std::uint64_t DomainHeavy = 0x48454156595F5052ULL;     // HEAVY_PR
constexpr std::uint64_t DomainAge = 0x535441525F414745ULL;       // STAR_AGE
constexpr double EpsilonTrace = 1.0 / 65536.0;

[[nodiscard]] double smooth(double t) noexcept {
    t = safe::finiteClamp(t, 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

[[nodiscard]] double lerp(double a, double b, double t) noexcept {
    return a + (b - a) * t;
}

[[nodiscard]] std::int64_t safeCell(double value) noexcept {
    if (!std::isfinite(value)) return 0;
    constexpr double lo = static_cast<double>(std::numeric_limits<std::int64_t>::lowest() / 4);
    constexpr double hi = static_cast<double>(std::numeric_limits<std::int64_t>::max() / 4);
    return static_cast<std::int64_t>(std::floor(safe::finiteClamp(value, lo, hi)));
}

[[nodiscard]] std::uint64_t hashCell(std::uint64_t seed, std::int64_t x, std::int64_t y, std::int64_t z) noexcept {
    auto h = mix64(seed ^ std::uint64_t(x));
    h = mix64(h ^ std::uint64_t(y));
    h = mix64(h ^ std::uint64_t(z));
    return h;
}

[[nodiscard]] double hashUnit(std::uint64_t h) noexcept {
    return static_cast<double>((mix64(h) >> 11U) & ((1ULL << 53U) - 1ULL)) /
           static_cast<double>(1ULL << 53U);
}

[[nodiscard]] double valueNoise(std::uint64_t seed, double x, double y, double z) noexcept {
    const std::int64_t ix = safeCell(x);
    const std::int64_t iy = safeCell(y);
    const std::int64_t iz = safeCell(z);
    const double fx = smooth(x - static_cast<double>(ix));
    const double fy = smooth(y - static_cast<double>(iy));
    const double fz = smooth(z - static_cast<double>(iz));

    std::array<double, 8> v{};
    std::size_t k = 0;
    for (int dz = 0; dz <= 1; ++dz) {
        for (int dy = 0; dy <= 1; ++dy) {
            for (int dx = 0; dx <= 1; ++dx) {
                v[k++] = hashUnit(hashCell(seed, ix + dx, iy + dy, iz + dz));
            }
        }
    }
    const double x00 = lerp(v[0], v[1], fx);
    const double x10 = lerp(v[2], v[3], fx);
    const double x01 = lerp(v[4], v[5], fx);
    const double x11 = lerp(v[6], v[7], fx);
    const double y0 = lerp(x00, x10, fy);
    const double y1 = lerp(x01, x11, fy);
    return safe::finiteClamp(lerp(y0, y1, fz), 0.0, 1.0);
}

[[nodiscard]] double sanitizeMultiplier(double value) noexcept {
    return safe::nonNegative(value);
}

} // namespace

UniverseFormationFields::UniverseFormationFields(std::uint64_t universeSeed, FormationTuning tuning)
    : seed_(universeSeed) {
    setTuning(tuning);
}

void UniverseFormationFields::setTuning(FormationTuning tuning) noexcept {
    tuning.starFormationMultiplier = sanitizeMultiplier(tuning.starFormationMultiplier);
    tuning.planetFormationMultiplier = sanitizeMultiplier(tuning.planetFormationMultiplier);
    tuning.resourceAbundanceMultiplier = sanitizeMultiplier(tuning.resourceAbundanceMultiplier);
    tuning_ = tuning;
}

std::uint64_t UniverseFormationFields::subSeed(
    std::uint64_t domain,
    std::uint64_t a,
    std::uint64_t b,
    std::uint64_t c) const noexcept {
    auto h = mix64(seed_ ^ mix64(domain));
    h = mix64(h ^ mix64(a));
    h = mix64(h ^ mix64(b));
    h = mix64(h ^ mix64(c));
    return h;
}

double UniverseFormationFields::unit(std::uint64_t seed) const noexcept {
    return hashUnit(seed);
}

double UniverseFormationFields::signedUnit(std::uint64_t seed) const noexcept {
    return safe::finiteClamp(unit(seed) * 2.0 - 1.0, -1.0, 1.0);
}

UniverseFormationFields::FieldProfile UniverseFormationFields::profile(
    std::uint64_t domain,
    std::uint64_t key) const {
    const auto s = subSeed(domain, key);
    FieldProfile p{};
    p.seed = s;
    // Shape parameters are all seed-derived. Fixed coefficients are algorithm-version
    // transforms, not per-universe authored values.
    p.frequency = std::exp(std::log(1.0e-5) + unit(s ^ 0x11ULL) * std::log(1000.0));
    p.amplitude = 0.65 + unit(s ^ 0x22ULL) * 1.35;
    p.persistence = 0.32 + unit(s ^ 0x33ULL) * 0.42;
    p.lacunarity = 1.55 + unit(s ^ 0x44ULL) * 1.75;
    p.octaves = static_cast<std::uint8_t>(2U + (mix64(s ^ 0x55ULL) % 6ULL));
    p.frequency = safe::nonNegative(p.frequency);
    p.amplitude = safe::nonNegative(p.amplitude);
    p.persistence = safe::finiteClamp(p.persistence, 0.0, 0.999999);
    p.lacunarity = safe::nonNegative(p.lacunarity);
    return p;
}

double UniverseFormationFields::field01(const FieldProfile& p, const UniversePoint& point) const {
    double frequency = p.frequency;
    double amplitude = 1.0;
    double sum = 0.0;
    double weight = 0.0;
    for (std::uint8_t octave = 0; octave < p.octaves; ++octave) {
        const auto os = mix64(p.seed ^ std::uint64_t(octave + 1U));
        const double phaseX = signedUnit(os ^ 0xA1ULL) * 4096.0;
        const double phaseY = signedUnit(os ^ 0xA2ULL) * 4096.0;
        const double phaseZ = signedUnit(os ^ 0xA3ULL) * 4096.0;
        const double n = valueNoise(
            os,
            point.xLy * frequency + phaseX,
            point.yLy * frequency + phaseY,
            point.zLy * frequency + phaseZ);
        sum = safe::nonNegative(sum + n * amplitude);
        weight = safe::nonNegative(weight + amplitude);
        frequency = safe::nonNegative(frequency * p.lacunarity);
        amplitude = safe::nonNegative(amplitude * p.persistence);
    }
    if (weight <= 0.0) return 0.0;
    return safe::finiteClamp((sum / weight) * p.amplitude, 0.0, safe::PublishedScalarCeiling);
}

double UniverseFormationFields::rawMaterialField(
    std::uint64_t materialKey,
    const UniversePoint& point) const {
    // materialKey participates in the profile seed, so every material has an
    // independent wavelength/phase/octave distribution rather than a shared ratio.
    return safe::nonNegative(field01(profile(DomainResource, materialKey), point));
}

StarFormationSample UniverseFormationFields::sampleStar(const UniversePoint& point) const {
    const double light = rawMaterialField(DomainLight, point);
    const double heavy = rawMaterialField(DomainHeavy, point);
    const double base = field01(profile(DomainStar), point);
    const double composition = safe::finiteClamp((light + heavy) * 0.5, 0.0, safe::PublishedScalarCeiling);

    // Star formation uses the unscaled seeded resource graph so the resource
    // balance multiplier does not silently alter stellar topology.
    const double raw = safe::nonNegative(base * (0.75 + 0.5 * composition));
    const double signal = safe::nonNegative(raw * tuning_.starFormationMultiplier);

    const auto ageProfile = profile(DomainAge);
    const double age = safe::finiteClamp(field01(ageProfile, point), 0.0, 1.0);
    const double metallicity = safe::finiteClamp(
        heavy / std::max(EpsilonTrace, light + heavy), 0.0, 1.0);

    return {
        .rawSignal = raw,
        .signal = signal,
        .primordialLight = safe::nonNegative(light),
        .primordialHeavy = safe::nonNegative(heavy),
        .age01 = age,
        .metallicity01 = metallicity,
        .exists = signal >= FormationFloor,
    };
}

PlanetFormationSample UniverseFormationFields::samplePlanet(
    std::uint64_t parentStarSeed,
    double parentStarSignal,
    std::uint64_t orbitCandidate) const {
    const auto s = subSeed(DomainPlanet, parentStarSeed, orbitCandidate);
    const double u0 = unit(s ^ 0x10ULL);
    const double u1 = unit(s ^ 0x20ULL);
    const double u2 = unit(s ^ 0x30ULL);
    const double u3 = unit(s ^ 0x40ULL);

    // Orbit spacing and world size are generated from the seed. No authored
    // planet-count ceiling exists: callers query whatever orbit candidates they need.
    const double seededBaseOrbit = std::exp(-2.1 + u0 * 2.4);
    const double seededSpacing = 0.55 + u1 * 1.15;
    const double indexScale = std::log1p(static_cast<double>(orbitCandidate));
    const double orbitalDistance = safe::nonNegative(
        seededBaseOrbit * std::exp(seededSpacing * std::sqrt(indexScale)));

    const double starInfluence = safe::finiteClamp(
        safe::nonNegative(parentStarSignal) / (FormationFloor + safe::nonNegative(parentStarSignal)),
        0.0,
        1.0);
    // Default occurrence is intentionally generous: many orbit candidates clear
    // the 1.0 floor, and the global planetFormationMultiplier can raise/lower it.
    const double localBias = safe::nonNegative(0.58 + u2 * 1.05 + starInfluence * (0.22 + u3 * 0.38));
    const double rawSignal = localBias;
    const double signal = safe::nonNegative(rawSignal * tuning_.planetFormationMultiplier);

    // Larger worlds trend farther from their star, with seed-derived strength.
    // The relation is statistical rather than absolute; local seeded variance can
    // still produce small distant worlds or large inner worlds.
    const double radiusSeed = unit(s ^ 0x50ULL);
    const double distanceCoupling = 0.12 + unit(s ^ 0x60ULL) * 0.72;
    // Planet physical scale is intentionally broad. The seed chooses the local baseline;
    // orbital distance only biases the result upward statistically.
    const double baseRadiusKm = 1800.0 * std::exp(radiusSeed * 4.35);
    const double distanceGrowth = 1.0 + std::log1p(orbitalDistance) * distanceCoupling;
    const double radiusKm = safe::nonNegative(baseRadiusKm * distanceGrowth);

    return {
        .orbitCandidate = orbitCandidate,
        .rawSignal = rawSignal,
        .signal = signal,
        .orbitalDistanceAu = orbitalDistance,
        .radiusKm = radiusKm,
        .localFormationBias = localBias,
        .exists = signal >= FormationFloor,
    };
}

ResourceFieldSample UniverseFormationFields::sampleResource(
    std::uint64_t materialKey,
    const UniversePoint& systemPoint,
    std::uint64_t planetSeed,
    const SpaceResourceContext& space) const {
    const double universal = rawMaterialField(materialKey, systemPoint);
    const auto local = subSeed(DomainResource, materialKey, planetSeed);
    const double localFactor = 0.35 + unit(local ^ 0xA1ULL) * 1.65;

    // Trace floor preserves "all planets contain all resources" when the global
    // resource multiplier is non-zero; independent material fields determine ratio.
    const double planetaryRaw = safe::nonNegative((EpsilonTrace + universal) * localFactor);
    const double planetary = safe::nonNegative(
        planetaryRaw * tuning_.resourceAbundanceMultiplier);

    const double starDistance = safe::nonNegative(space.distanceToNearestStarLy);
    const double planetDistance = safe::nonNegative(space.distanceToNearestPlanetAu);
    const double stellarScale = 0.15 + unit(local ^ 0xB1ULL) * 3.85;
    const double planetAvoidanceScale = 0.01 + unit(local ^ 0xB2ULL) * 0.49;

    // Space-source resources peak inside/near stellar systems but away from
    // planets. Interstellar void and planetary gravity-well proximity suppress
    // meteorite/minor-body density. This envelope NEVER feeds planetary ore.
    const double stellarSupport = space.insideStellarSystem
        ? 1.0
        : std::exp(-starDistance / std::max(EpsilonTrace, stellarScale));
    const double planetClearance = 1.0 - std::exp(
        -planetDistance / std::max(EpsilonTrace, planetAvoidanceScale));
    const double spaceRaw = safe::nonNegative(
        (EpsilonTrace + universal) * safe::finiteClamp(stellarSupport, 0.0, 1.0) *
        safe::finiteClamp(planetClearance, 0.0, 1.0));
    const double spaceDensity = safe::nonNegative(
        spaceRaw * tuning_.resourceAbundanceMultiplier);

    return {
        .materialKey = materialKey,
        .rawUniversalField = safe::nonNegative(universal),
        .planetaryAbundance = planetary,
        .spaceSourceDensity = spaceDensity,
    };
}

} // namespace elysium
