// Intended function: Generate deterministic item affixes from tier, item family, seed, alignment, rarity, exclusions, and power budget.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::loot {
struct AffixRoll {
    std::uint64_t rollId{};
    std::uint64_t itemId{};
    std::uint64_t affixId{};
    std::uint64_t tier{};
    double magnitude{};
    std::uint64_t seed{};
};
class AffixRollStore {
public:
 bool put(AffixRoll v); bool erase(std::uint64_t id);
 [[nodiscard]] const AffixRoll* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<AffixRoll>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const AffixRoll& v) noexcept; std::vector<AffixRoll> values_;
};
} // namespace elysium::loot
