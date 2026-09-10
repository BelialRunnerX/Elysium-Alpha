// Intended function: generate sparse star candidates and planet/resource context from stable seed fields.
#include "galaxy/GalaxyGenerator.hpp"

#include "core/Determinism.hpp"
#include "core/Saturating.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace elysium {
namespace {
constexpr double Pi = 3.14159265358979323846;
constexpr double TwoPi = Pi * 2.0;
constexpr double Epsilon = 1.0e-12;

double unit(std::uint64_t x) {
    return static_cast<double>((mix64(x) >> 11U) & ((1ULL << 53U) - 1ULL)) /
           static_cast<double>(1ULL << 53U);
}

double signedUnit(std::uint64_t x) {
    return safe::finiteClamp(unit(x) * 2.0 - 1.0, -1.0, 1.0);
}

StarClass starFromComposition(const StarFormationSample& sample, std::uint64_t seed) {
    const double r = unit(seed ^ 0x77ULL);
    const double age = safe::finiteClamp(sample.age01, 0.0, 1.0);
    const double metals = safe::finiteClamp(sample.metallicity01, 0.0, 1.0);
    const double light = safe::nonNegative(sample.primordialLight);
    const double heavy = safe::nonNegative(sample.primordialHeavy);

    // Stellar class boundaries depend on the local universal resource composition,
    // stellar-age field and a residual per-star sub-seed. This avoids a global fixed
    // proportion table while still producing stable broad astrophysical categories.
    const double compositionSkew = safe::finiteClamp((heavy - light) / std::max(Epsilon, heavy + light), -1.0, 1.0);
    const double hotBias = safe::finiteClamp((1.0 - age) * 0.18 + metals * 0.09 + compositionSkew * 0.04, 0.0, 0.32);
    const double remnantBias = safe::finiteClamp(age * (0.08 + std::max(0.0, compositionSkew) * 0.08), 0.0, 0.20);
    if (r < 0.40 - hotBias * 0.35) return StarClass::M;
    if (r < 0.64 - hotBias * 0.20) return StarClass::K;
    if (r < 0.80 - hotBias * 0.08) return StarClass::G;
    if (r < 0.89 + hotBias * 0.25) return StarClass::F;
    if (r < 0.95 + hotBias * 0.30) return StarClass::A;
    if (r < 0.982 + remnantBias * 0.08) return StarClass::WhiteDwarf;
    if (r < 0.997 + remnantBias * 0.015) return StarClass::Neutron;
    return StarClass::Exotic;
}
} // namespace

GalaxyGenerator::GalaxyGenerator(std::uint64_t galaxySeed, FormationTuning tuning)
    : seed_(galaxySeed), formation_(galaxySeed, tuning), shape_(deriveShape()) {}

GalaxyShapeProfile GalaxyGenerator::deriveShape() const {
    const auto s = mix64(seed_ ^ 0x47414C4158595348ULL); // GALAXYSH
    GalaxyShapeProfile p{};
    // Seed-derived exponential scales mean there is no authored outer galaxy radius.
    // The distribution has a long tail; numeric safety is the only hard envelope.
    p.radialScaleLy = safe::nonNegative(6'000.0 * std::exp(unit(s ^ 0x11ULL) * 2.65));
    p.verticalScaleLy = safe::nonNegative(180.0 * std::exp(unit(s ^ 0x22ULL) * 3.05));
    p.twistRadiansPerScale = safe::nonNegative(1.4 + unit(s ^ 0x33ULL) * 8.5);
    p.armScatterRadians = safe::nonNegative(0.08 + unit(s ^ 0x44ULL) * 0.52);
    p.interArmScatterRadians = safe::nonNegative(0.55 + unit(s ^ 0x55ULL) * 2.20);
    p.interArmFraction = safe::finiteClamp(0.08 + unit(s ^ 0x66ULL) * 0.52, 0.0, 1.0);
    p.armCount = static_cast<std::uint32_t>(2ULL + (mix64(s ^ 0x77ULL) % 7ULL));
    return p;
}

std::uint64_t GalaxyGenerator::systemSeed(GalaxySystemId index) const {
    return mix64(seed_ ^ mix64(index * 0x9E3779B97F4A7C15ULL));
}

std::uint64_t GalaxyGenerator::planetSeed(GalaxySystemId index, PlanetCandidateId orbitCandidate) const {
    return mix64(systemSeed(index) ^ mix64((orbitCandidate + 1ULL) * 0xD1B54A32D192ED03ULL));
}

UniversePoint GalaxyGenerator::candidatePoint(GalaxySystemId index) const {
    const auto s = systemSeed(index);
    // Exponential disk radius: no fixed outer radius. u never reaches 1 exactly.
    const double uRadial = safe::finiteClamp(unit(s ^ 0x11ULL), 0.0, 1.0 - Epsilon);
    const double radial = safe::nonNegative(-std::log1p(-uRadial) * shape_.radialScaleLy);
    const bool interArm = unit(s ^ 0x22ULL) < shape_.interArmFraction;
    const std::uint32_t arm = static_cast<std::uint32_t>(mix64(s ^ 0x33ULL) % std::max<std::uint32_t>(1U, shape_.armCount));
    const double armStep = TwoPi / static_cast<double>(std::max<std::uint32_t>(1U, shape_.armCount));
    const double baseAngle = static_cast<double>(arm) * armStep +
        (radial / std::max(Epsilon, shape_.radialScaleLy)) * shape_.twistRadiansPerScale;
    const double scatterScale = interArm ? shape_.interArmScatterRadians : shape_.armScatterRadians;
    const double angle = baseAngle + signedUnit(s ^ 0x44ULL) * scatterScale;

    // A gaussian-like vertical coordinate from two seeded uniforms. The vertical
    // scale is seed-derived and there is no authored clipping height.
    const double u1 = std::max(Epsilon, unit(s ^ 0x55ULL));
    const double u2 = unit(s ^ 0x56ULL);
    const double normal = std::sqrt(-2.0 * std::log(u1)) * std::cos(TwoPi * u2);
    const double verticalTaper = 1.0 / std::sqrt(1.0 + radial / std::max(Epsilon, shape_.radialScaleLy));
    const double y = normal * shape_.verticalScaleLy * verticalTaper;

    return {
        .xLy = safe::finiteClamp(std::cos(angle) * radial, -safe::PublishedScalarCeiling, safe::PublishedScalarCeiling),
        .yLy = safe::finiteClamp(y, -safe::PublishedScalarCeiling, safe::PublishedScalarCeiling),
        .zLy = safe::finiteClamp(std::sin(angle) * radial, -safe::PublishedScalarCeiling, safe::PublishedScalarCeiling),
    };
}

GalaxySystemDescriptor GalaxyGenerator::describe(GalaxySystemId index) const {
    const auto s = systemSeed(index);
    const auto point = candidatePoint(index);
    const auto star = formation_.sampleStar(point);
    const auto firstPlanet = formation_.samplePlanet(s, star.signal, 0);

    GalaxySystemDescriptor d{};
    d.systemIndex = index;
    d.regionIndex = regionOf(index);
    d.xLy = point.xLy;
    d.yLy = point.yLy;
    d.zLy = point.zLy;
    d.starPresent = star.exists;
    d.starFormationSignal = safe::nonNegative(star.signal);
    d.planetFormationBaseline = safe::nonNegative(firstPlanet.signal);
    d.starClass = starFromComposition(star, s);
    d.stellarAge01 = static_cast<float>(safe::finiteClamp(star.age01, 0.0, 1.0));
    d.metallicity01 = static_cast<float>(safe::finiteClamp(star.metallicity01, 0.0, 1.0));

    // Political/resource display projections derive from local seed fields rather than
    // imposing a galaxy edge. They are presentation/query hints, not existence gates.
    const double distanceScale = safe::nonNegative(
        std::sqrt(point.xLy * point.xLy + point.zLy * point.zLy) /
        std::max(Epsilon, shape_.radialScaleLy));
    d.imperialPressure = static_cast<float>(safe::finiteClamp(
        std::exp(-distanceScale) * (0.72 + unit(s ^ 0xA1ULL) * 0.38), 0.0, 1.0));
    d.resourceRichness = static_cast<float>(safe::finiteClamp(
        0.10 + static_cast<double>(d.metallicity01) * 0.70 + unit(s ^ 0xA2ULL) * 0.35,
        0.0, 1.0));
    return d;
}

PlanetFormationSample GalaxyGenerator::describePlanet(
    GalaxySystemId index,
    PlanetCandidateId orbitCandidate) const {
    const auto starDescriptor = describe(index);
    return formation_.samplePlanet(systemSeed(index), starDescriptor.starFormationSignal, orbitCandidate);
}

PlanetCandidateDescriptor GalaxyGenerator::planetCandidate(
    GalaxySystemId system,
    PlanetCandidateId candidate) const {
    return {
        .systemId = system,
        .candidateId = candidate,
        .stableSeed = planetSeed(system, candidate),
        .formation = describePlanet(system, candidate),
    };
}

std::vector<PlanetCandidateDescriptor> GalaxyGenerator::existingPlanetCandidates(
    GalaxySystemId system,
    PlanetCandidateId firstCandidate,
    std::size_t candidatesToInspect,
    std::size_t maxResults) const {
    std::vector<PlanetCandidateDescriptor> out;
    if (candidatesToInspect == 0) return out;
    out.reserve(maxResults == 0 ? std::min<std::size_t>(candidatesToInspect, 32) : std::min(maxResults, candidatesToInspect));
    PlanetCandidateId candidate = firstCandidate;
    for (std::size_t inspected = 0; inspected < candidatesToInspect; ++inspected) {
        const auto descriptor = planetCandidate(system, candidate);
        if (descriptor.formation.exists) {
            out.push_back(descriptor);
            if (maxResults != 0 && out.size() >= maxResults) break;
        }
        if (candidate == std::numeric_limits<PlanetCandidateId>::max()) break;
        ++candidate;
    }
    return out;
}

std::vector<GalaxySystemId> GalaxyGenerator::sampleRegion(
    GalaxyRegionId region,
    std::size_t maxCount) const {
    std::vector<GalaxySystemId> out;
    const GalaxySystemId begin = firstSystemOf(region);
    const std::size_t wanted = std::min<std::size_t>(maxCount, static_cast<std::size_t>(SystemsPerAddressRegion));
    out.reserve(wanted);
    for (std::size_t i = 0; i < wanted; ++i) {
        if (begin > MaxSystemId - static_cast<GalaxySystemId>(i)) break;
        out.push_back(begin + static_cast<GalaxySystemId>(i));
    }
    return out;
}

} // namespace elysium
