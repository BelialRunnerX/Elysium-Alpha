// Intended function: Generate deterministic named galaxy regions, arm/interarm classification, Imperial pressure, resources, hazards, and route density.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::procedural {
struct RegionSeed {
    std::uint64_t regionId{};
    std::uint64_t seed{};
    std::uint64_t armClass{};
    double imperialPressure{};
    double resourceScore{};
    double hazardScore{};
};
class RegionSeedIndex {
public:
 bool upsert(RegionSeed value); bool erase(std::uint64_t id); [[nodiscard]] const RegionSeed* find(std::uint64_t id) const; [[nodiscard]] const std::vector<RegionSeed>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const RegionSeed& value) noexcept; std::vector<RegionSeed> rows_;
};
}
