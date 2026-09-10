// Intended function: Generate deterministic planet descriptors, class, radius, gravity, atmosphere, resource/climate fields, moons, and generator identity.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::procedural {
struct PlanetSeed {
    std::uint64_t planetId{};
    std::uint64_t seed{};
    std::uint64_t planetClass{};
    double radius{};
    double gravity{};
    std::uint64_t featureHash{};
};
class PlanetSeedIndex {
public:
 bool upsert(PlanetSeed value); bool erase(std::uint64_t id); [[nodiscard]] const PlanetSeed* find(std::uint64_t id) const; [[nodiscard]] const std::vector<PlanetSeed>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const PlanetSeed& value) noexcept; std::vector<PlanetSeed> rows_;
};
}
