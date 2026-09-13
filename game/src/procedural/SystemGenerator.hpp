// Intended function: Generate deterministic star-system descriptors, star classes, planets, belts, stations, anomalies, routes, and faction pressure.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::procedural {
struct SystemSeed {
    std::uint64_t systemId{};
    std::uint64_t seed{};
    std::uint64_t starClass{};
    std::uint64_t planetCount{};
    std::uint64_t featureHash{};
    std::uint64_t factionPressure{};
};
class SystemSeedIndex {
public:
 bool upsert(SystemSeed value); bool erase(std::uint64_t id); [[nodiscard]] const SystemSeed* find(std::uint64_t id) const; [[nodiscard]] const std::vector<SystemSeed>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const SystemSeed& value) noexcept; std::vector<SystemSeed> rows_;
};
}
