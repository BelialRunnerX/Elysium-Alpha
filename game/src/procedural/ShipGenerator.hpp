// Intended function: Generate deterministic ship variants from hull archetype, manufacturer/faction style, modules, wear, cargo role, and history.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::procedural {
struct ShipSeed {
    std::uint64_t shipId{};
    std::uint64_t seed{};
    double hullId{};
    std::uint64_t factionId{};
    std::uint64_t roleId{};
    std::uint64_t tier{};
};
class ShipSeedTable {
public:
 bool set(ShipSeed value); bool remove(std::uint64_t id);
 [[nodiscard]] const ShipSeed* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<ShipSeed> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const ShipSeed& value) noexcept; std::vector<ShipSeed> rows_;
};
}
