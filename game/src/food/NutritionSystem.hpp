// Intended function: Track food nutrition, hydration, spoilage, contamination, preferences, dietary restrictions, and meal quality.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::food {
struct NutritionRecord {
    std::uint64_t foodId{};
    double calories{};
    double hydration{};
    double quality{};
    double spoilage{};
    double hazard{};
};
class NutritionRecordStore {
public:
 bool put(NutritionRecord v); bool erase(std::uint64_t id);
 [[nodiscard]] const NutritionRecord* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<NutritionRecord>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const NutritionRecord& v) noexcept; std::vector<NutritionRecord> values_;
};
} // namespace elysium::food
