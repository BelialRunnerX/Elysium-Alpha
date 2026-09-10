// Intended function: Generate deterministic settlement site plans, districts, infrastructure anchors, resource context, faction, and history seeds.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::procedural {
struct SettlementSeed {
    std::uint64_t siteId{};
    std::uint64_t seed{};
    std::uint64_t factionId{};
    std::uint64_t populationTier{};
    std::uint64_t industryTier{};
    std::uint64_t hazardTier{};
};
class SettlementSeedTable {
public:
 bool set(SettlementSeed value); bool remove(std::uint64_t id);
 [[nodiscard]] const SettlementSeed* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<SettlementSeed> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const SettlementSeed& value) noexcept; std::vector<SettlementSeed> rows_;
};
}
