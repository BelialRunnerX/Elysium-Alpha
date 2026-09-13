// Intended function: seed-authoritative universe formation and independent per-resource abundance fields.
#pragma once

#include <cstdint>
#include <span>

namespace elysium {

struct UniversePoint {
    double xLy{};
    double yLy{};
    double zLy{};
};

struct FormationTuning {
    // Explicit balance knobs. These are intentionally NOT seed-derived.
    // 2.0 doubles the corresponding field output before final saturation.
    double starFormationMultiplier{1.0};
    double planetFormationMultiplier{1.0};
    double resourceAbundanceMultiplier{1.0};
};

struct StarFormationSample {
    double rawSignal{};       // seeded field before balance multiplier
    double signal{};          // published, clamped signal
    double primordialLight{}; // seeded primordial resource field
    double primordialHeavy{}; // seeded primordial resource field
    double age01{};           // seeded stellar age characteristic
    double metallicity01{};   // derived from primordial resource fields
    bool exists{};            // signal >= FormationFloor
};

struct PlanetFormationSample {
    std::uint64_t orbitCandidate{};
    double rawSignal{};
    double signal{};
    double orbitalDistanceAu{};
    double radiusKm{};
    double localFormationBias{};
    bool exists{};
};

struct ResourceFieldSample {
    std::uint64_t materialKey{};
    double rawUniversalField{}; // independent per material, no balance multiplier
    double planetaryAbundance{};
    double spaceSourceDensity{};
};

struct SpaceResourceContext {
    // Distance from the sample point to the nearest star and planet.
    // Planet distance is only used for space-source suppression and never
    // changes planetary abundance.
    double distanceToNearestStarLy{};
    double distanceToNearestPlanetAu{};
    bool insideStellarSystem{false};
};

class UniverseFormationFields final {
public:
    // FormationFloor is a versioned algorithm boundary, not a population cap.
    static constexpr double FormationFloor = 1.0;

    explicit UniverseFormationFields(std::uint64_t universeSeed, FormationTuning tuning = {});

    [[nodiscard]] std::uint64_t universeSeed() const noexcept { return seed_; }
    [[nodiscard]] const FormationTuning& tuning() const noexcept { return tuning_; }
    void setTuning(FormationTuning tuning) noexcept;

    [[nodiscard]] StarFormationSample sampleStar(const UniversePoint& point) const;
    [[nodiscard]] PlanetFormationSample samplePlanet(
        std::uint64_t parentStarSeed,
        double parentStarSignal,
        std::uint64_t orbitCandidate) const;

    // Every material key receives a different deterministic field profile.
    // With resourceAbundanceMultiplier > 0, every planet retains a non-zero
    // trace abundance; relative proportions vary independently by material.
    [[nodiscard]] ResourceFieldSample sampleResource(
        std::uint64_t materialKey,
        const UniversePoint& systemPoint,
        std::uint64_t planetSeed,
        const SpaceResourceContext& space) const;

    [[nodiscard]] double rawMaterialField(
        std::uint64_t materialKey,
        const UniversePoint& point) const;

    // Stable sub-seeds are the required source for non-balance procedural values.
    [[nodiscard]] std::uint64_t subSeed(
        std::uint64_t domain,
        std::uint64_t a = 0,
        std::uint64_t b = 0,
        std::uint64_t c = 0) const noexcept;

private:
    struct FieldProfile {
        std::uint64_t seed{};
        double frequency{};
        double amplitude{};
        double persistence{};
        double lacunarity{};
        std::uint8_t octaves{};
    };

    [[nodiscard]] FieldProfile profile(std::uint64_t domain, std::uint64_t key = 0) const;
    [[nodiscard]] double field01(const FieldProfile& profile, const UniversePoint& point) const;
    [[nodiscard]] double unit(std::uint64_t seed) const noexcept;
    [[nodiscard]] double signedUnit(std::uint64_t seed) const noexcept;

    std::uint64_t seed_{};
    FormationTuning tuning_{};
};

} // namespace elysium
