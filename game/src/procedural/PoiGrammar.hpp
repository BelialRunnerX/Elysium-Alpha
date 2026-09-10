// Intended function: Compose deterministic POI kits from entrance/core/support/loot/hazard/story modules with stable generated object IDs.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::procedural {
struct PoiGrammarState {
    std::uint64_t grammarId{};
    std::uint64_t seed{};
    std::uint64_t moduleHash{};
    double difficulty{};
    double rarity{};
    std::uint64_t flags{};
};
class PoiGrammarStateIndex {
public:
 bool upsert(PoiGrammarState value); bool erase(std::uint64_t id); [[nodiscard]] const PoiGrammarState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<PoiGrammarState>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const PoiGrammarState& value) noexcept; std::vector<PoiGrammarState> rows_;
};
}
