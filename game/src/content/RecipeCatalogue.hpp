// Intended function: Provide stable recipe definitions with ingredients, outputs, workstation tags, unlocks, and processing costs.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::content {
struct RecipeRecord {
    std::uint64_t recipeId{};
    std::uint64_t workstationTag{};
    std::uint64_t inputHash{};
    std::uint64_t outputHash{};
    double workCost{};
    std::uint64_t unlockId{};
};
class RecipeRecordRegistry {
public:
    bool publish(RecipeRecord record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const RecipeRecord* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<RecipeRecord>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const RecipeRecord& r) noexcept;
    std::vector<RecipeRecord> records_;
};
} // namespace elysium::content
