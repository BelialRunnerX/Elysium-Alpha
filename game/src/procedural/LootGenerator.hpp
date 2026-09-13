// Intended function: Generate deterministic loot bundles from loot tables, level/tier, biome/faction, rarity budget, uniqueness, and provenance policy.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::procedural {
struct LootGenerationState {
    std::uint64_t rollId{};
    std::uint64_t seed{};
    std::uint64_t tableId{};
    std::uint64_t tier{};
    double budget{};
    std::uint64_t flags{};
};
class LootGenerationStateIndex {
public:
 bool upsert(LootGenerationState value); bool erase(std::uint64_t id); [[nodiscard]] const LootGenerationState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<LootGenerationState>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const LootGenerationState& value) noexcept; std::vector<LootGenerationState> rows_;
};
}
