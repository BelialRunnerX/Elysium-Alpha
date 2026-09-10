// Intended function: Compose deterministic dungeon room graphs with guaranteed connectivity, loops, locks, keys, hazards, and encounter pacing.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::procedural {
struct DungeonGrammarState {
    std::uint64_t grammarId{};
    std::uint64_t seed{};
    double roomBudget{};
    double loopBudget{};
    double lockBudget{};
    double difficulty{};
};
class DungeonGrammarStateIndex {
public:
 bool upsert(DungeonGrammarState value); bool erase(std::uint64_t id); [[nodiscard]] const DungeonGrammarState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<DungeonGrammarState>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const DungeonGrammarState& value) noexcept; std::vector<DungeonGrammarState> rows_;
};
}
