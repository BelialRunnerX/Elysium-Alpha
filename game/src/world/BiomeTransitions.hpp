// Intended function: Resolve biome boundaries/blends from climate, geology, elevation, water, disturbance, and authored overrides.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::world {
struct BiomeBlend {
    std::uint64_t sampleId{};
    std::uint64_t primaryBiome{};
    std::uint64_t secondaryBiome{};
    double blend{};
    double disturbance{};
    std::uint64_t flags{};
};
class BiomeBlendStore {
public:
 bool put(BiomeBlend v); bool erase(std::uint64_t id);
 [[nodiscard]] const BiomeBlend* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<BiomeBlend>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const BiomeBlend& v) noexcept; std::vector<BiomeBlend> values_;
};
} // namespace elysium::world
