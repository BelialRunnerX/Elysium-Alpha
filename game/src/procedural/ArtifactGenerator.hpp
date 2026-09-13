// Intended function: Generate unique artifacts from maker, culture, materials, obsession/event context, function, imagery, and provenance seed.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::procedural {
struct ArtifactSeed {
    std::uint64_t artifactId{};
    std::uint64_t seed{};
    std::uint64_t makerId{};
    std::uint64_t cultureId{};
    std::uint64_t materialHash{};
    std::uint64_t eventId{};
};
class ArtifactSeedTable {
public:
 bool set(ArtifactSeed value); bool remove(std::uint64_t id);
 [[nodiscard]] const ArtifactSeed* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<ArtifactSeed> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const ArtifactSeed& value) noexcept; std::vector<ArtifactSeed> rows_;
};
}
