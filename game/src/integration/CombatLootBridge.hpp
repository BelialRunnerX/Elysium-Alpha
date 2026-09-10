// Intended function: Translate committed actor deaths/destruction into deterministic loot-roll, provenance, salvage, and history requests.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::integration {
struct CombatLootIntent {
    std::uint64_t intentId{};
    std::uint64_t victimId{};
    std::uint64_t killerId{};
    std::uint64_t lootTableId{};
    std::uint64_t seed{};
    std::uint64_t flags{};
};
class CombatLootIntentIndex {
public:
 bool upsert(CombatLootIntent value); bool erase(std::uint64_t id); [[nodiscard]] const CombatLootIntent* find(std::uint64_t id) const; [[nodiscard]] const std::vector<CombatLootIntent>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const CombatLootIntent& value) noexcept; std::vector<CombatLootIntent> rows_;
};
}
