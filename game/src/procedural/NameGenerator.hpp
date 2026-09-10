// Intended function: Generate deterministic star, planet, region, site, faction, person, ship, artifact, and creature names from labeled seed streams.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::procedural {
struct NameSeed {
    std::uint64_t nameId{};
    std::uint64_t seed{};
    std::uint64_t category{};
    std::uint64_t cultureId{};
    std::uint64_t syllableSet{};
    std::uint64_t flags{};
};
class NameSeedTable {
public:
 bool set(NameSeed value); bool remove(std::uint64_t id);
 [[nodiscard]] const NameSeed* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<NameSeed> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const NameSeed& value) noexcept; std::vector<NameSeed> rows_;
};
}
