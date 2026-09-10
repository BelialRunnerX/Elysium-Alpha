// Intended function: generate sparse deterministic ore-vein descriptors and depletion records without planet-wide dense allocation.
#pragma once

#include "galaxy/UniverseFormationFields.hpp"
#include "world/PlanetResourceField.hpp"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <vector>

namespace elysium::world {

struct OreVein {
    std::uint64_t stableId{};
    std::uint64_t oreItemId{};
    double grade{};
    std::uint64_t remainingUnits{};
    double radius{};
    double depth{};
};

struct OreVeinCandidate {
    std::uint64_t candidateKey{};
    std::uint64_t materialKey{};
    elysium::PlanetResourcePoint location{};
};

struct GeneratedOreVein {
    bool present{};
    OreVein vein{};
    double localAbundance{};
    double probability{};
};

class OreVeinGenerator final {
public:
    explicit OreVeinGenerator(const elysium::UniverseFormationFields& fields) : fields_(&fields), local_(fields) {}

    [[nodiscard]] GeneratedOreVein probe(
        const elysium::UniversePoint& systemPoint,
        std::uint64_t planetSeed,
        const OreVeinCandidate& candidate) const;

private:
    const elysium::UniverseFormationFields* fields_{};
    elysium::PlanetResourceField local_;
};

class OreVeinStore {
public:
    bool upsert(OreVein value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const OreVein* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<OreVein> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const OreVein& value) noexcept;
    std::vector<OreVein> records_;
};

} // namespace elysium::world
