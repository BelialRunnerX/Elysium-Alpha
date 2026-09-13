// Intended function: deterministic spherical climate/height/density generation with no dense planet allocation.
#include "world/PlanetFieldGenerator.hpp"

#include "core/Determinism.hpp"
#include "core/Saturating.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace elysium {
namespace {
constexpr double Epsilon = 1.0e-12;

double smooth(double t) noexcept {
    t = safe::finiteClamp(t, 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

double unit(std::uint64_t h) noexcept {
    return static_cast<double>((mix64(h) >> 11U) & ((1ULL << 53U) - 1ULL)) /
           static_cast<double>(1ULL << 53U);
}

std::int64_t floorCell(double v) noexcept {
    if (!std::isfinite(v)) return 0;
    constexpr double lo = -9.0e15;
    constexpr double hi = 9.0e15;
    return static_cast<std::int64_t>(std::floor(safe::finiteClamp(v, lo, hi)));
}

double lerp(double a, double b, double t) noexcept { return a + (b - a) * t; }
} // namespace

PlanetFieldGenerator::PlanetFieldGenerator(
    std::uint64_t universeSeed,
    PlanetPhysicalProfile profile,
    PlanetScaleDescriptor scale)
    : universeSeed_(universeSeed), profile_(profile), scale_(scale) {
    profile_.radiusKm = safe::nonNegative(profile_.radiusKm);
    scale_.radiusMeters = safe::nonNegative(scale_.radiusMeters);
}

std::uint64_t PlanetFieldGenerator::sub(std::uint64_t domain, std::uint64_t extra) const noexcept {
    auto h = mix64(universeSeed_ ^ mix64(domain));
    h = mix64(h ^ mix64(profile_.stableSeed));
    h = mix64(h ^ mix64(extra));
    return h;
}

double PlanetFieldGenerator::noise3(
    std::uint64_t seed,
    double x,
    double y,
    double z) const noexcept {
    const auto ix = floorCell(x), iy = floorCell(y), iz = floorCell(z);
    const double fx = smooth(x - static_cast<double>(ix));
    const double fy = smooth(y - static_cast<double>(iy));
    const double fz = smooth(z - static_cast<double>(iz));
    std::array<double, 8> v{};
    std::size_t n = 0;
    for (int dz = 0; dz <= 1; ++dz) for (int dy = 0; dy <= 1; ++dy) for (int dx = 0; dx <= 1; ++dx) {
        auto h = mix64(seed ^ mix64(static_cast<std::uint64_t>(ix + dx)));
        h = mix64(h ^ mix64(static_cast<std::uint64_t>(iy + dy)));
        h = mix64(h ^ mix64(static_cast<std::uint64_t>(iz + dz)));
        v[n++] = unit(h);
    }
    const double x00 = lerp(v[0], v[1], fx), x10 = lerp(v[2], v[3], fx);
    const double x01 = lerp(v[4], v[5], fx), x11 = lerp(v[6], v[7], fx);
    return safe::finiteClamp(lerp(lerp(x00, x10, fy), lerp(x01, x11, fy), fz), 0.0, 1.0);
}

double PlanetFieldGenerator::fractal(
    std::uint64_t seed,
    Vec3 direction,
    double scale,
    std::uint32_t octaves) const noexcept {
    direction = normalize(direction);
    double sum = 0.0, weight = 0.0, amp = 1.0, freq = std::max(Epsilon, safe::nonNegative(scale));
    const std::uint32_t count = std::clamp<std::uint32_t>(octaves, 1U, 12U);
    for (std::uint32_t i = 0; i < count; ++i) {
        const auto os = mix64(seed ^ i);
        const double px = (unit(os ^ 0x11ULL) - 0.5) * 1024.0;
        const double py = (unit(os ^ 0x22ULL) - 0.5) * 1024.0;
        const double pz = (unit(os ^ 0x33ULL) - 0.5) * 1024.0;
        sum += noise3(os,
            static_cast<double>(direction.x) * freq + px,
            static_cast<double>(direction.y) * freq + py,
            static_cast<double>(direction.z) * freq + pz) * amp;
        weight += amp;
        freq = safe::nonNegative(freq * (1.7 + unit(os ^ 0x44ULL) * 0.95));
        amp *= 0.35 + unit(os ^ 0x55ULL) * 0.35;
    }
    return safe::finiteClamp(weight > 0.0 ? sum / weight : 0.0, 0.0, 1.0);
}

PlanetFieldSample PlanetFieldGenerator::sample(Vec3 direction, double radialOffsetMeters) const noexcept {
    direction = normalize(direction);
    if (lengthSq(direction) < 0.5f) direction = {0.0f, 1.0f, 0.0f};

    const auto climateSeed = sub(0x434C494D415445ULL); // CLIMATE
    const auto heightSeed  = sub(0x484549474854ULL);   // HEIGHT
    const auto ridgeSeed   = sub(0x5249444745ULL);     // RIDGE
    const auto caveSeed    = sub(0x4341564553ULL);     // CAVES
    const auto tectSeed    = sub(0x544543544F4EULL);   // TECTON

    const double seedScale = 0.8 + unit(sub(0x5343414C45ULL)) * 6.2;
    const double latitude = std::abs(static_cast<double>(direction.y));
    const double climate = fractal(climateSeed, direction, seedScale, 5);
    const double moistureNoise = fractal(climateSeed ^ 0xA5ULL, direction, seedScale * 0.73, 5);
    const double tectonic = fractal(tectSeed, direction, seedScale * 1.8, 6);
    const double erosion = fractal(heightSeed ^ 0xE1ULL, direction, seedScale * 2.4, 5);
    const double ridgeBase = fractal(ridgeSeed, direction, seedScale * 4.0, 6);
    const double ridge = safe::finiteClamp(1.0 - std::abs(ridgeBase * 2.0 - 1.0), 0.0, 1.0);

    const double temperature = safe::finiteClamp(
        profile_.irradiation / (1.0 + profile_.irradiation) * 0.60 + climate * 0.40 - latitude * 0.36,
        0.0, 1.0);
    const double moisture = safe::finiteClamp(
        profile_.volatilePotential * 0.48 + profile_.oceanPotential * 0.28 + moistureNoise * 0.44,
        0.0, 1.0);
    const double radiation = safe::finiteClamp(
        profile_.radiationPotential * 0.70 + fractal(climateSeed ^ 0xC3ULL, direction, seedScale * 0.4, 3) * 0.30,
        0.0, 1.0);

    // Relief grows sublinearly with planet radius so large planets become truly huge
    // without every mountain becoming a fixed percentage of the radius.
    const double radius = std::max(1.0, scale_.radiusMeters);
    const double reliefScale = safe::nonNegative(
        std::pow(radius, 0.55) * (0.45 + unit(sub(0x52454C494546ULL)) * 1.25));
    const double continent = fractal(heightSeed, direction, seedScale * 0.55, 6) * 2.0 - 1.0;
    const double fine = fractal(heightSeed ^ 0xF2ULL, direction, seedScale * 7.5, 5) * 2.0 - 1.0;
    const double height = safe::finiteClamp(
        continent * reliefScale * (0.65 + tectonic * 0.70) +
        ridge * reliefScale * 0.55 - erosion * reliefScale * 0.28 + fine * reliefScale * 0.12,
        -safe::PublishedScalarCeiling, safe::PublishedScalarCeiling);

    // Radial field for caves/overhangs. Density positive means matter. The cave
    // modulation is direction+altitude continuous and never needs a global voxel array.
    const double normalizedAltitude = safe::finiteClamp(
        radialOffsetMeters / std::max(1.0, reliefScale), -64.0, 64.0);
    const double caveFreq = 3.0 + unit(sub(0x4341564546524551ULL)) * 9.0;
    const double cave = noise3(caveSeed,
        static_cast<double>(direction.x) * caveFreq + normalizedAltitude * 0.37,
        static_cast<double>(direction.y) * caveFreq + normalizedAltitude * 0.53,
        static_cast<double>(direction.z) * caveFreq + normalizedAltitude * 0.71);
    const double overhang = (cave - 0.5) * reliefScale * (0.08 + tectonic * 0.14);
    const double density = safe::finiteClamp(
        height - radialOffsetMeters + overhang,
        -safe::PublishedScalarCeiling, safe::PublishedScalarCeiling);

    return {
        .temperature01 = temperature,
        .moisture01 = moisture,
        .radiation01 = radiation,
        .tectonic01 = tectonic,
        .erosion01 = erosion,
        .ridge01 = ridge,
        .surfaceHeightMeters = height,
        .signedDensity = density,
    };
}

} // namespace elysium
