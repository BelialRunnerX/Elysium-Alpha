// Intended function: Represent kitchen recipes, ingredient substitutions, cook skill, meal quality, batch size, and contamination handling.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::food {
struct CookingJob {
    std::uint64_t jobId{};
    std::uint64_t recipeId{};
    std::uint64_t cookId{};
    double progress{};
    double quality{};
    std::uint64_t state{};
};
class CookingJobStore {
public:
 bool put(CookingJob v); bool erase(std::uint64_t id);
 [[nodiscard]] const CookingJob* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<CookingJob>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const CookingJob& v) noexcept; std::vector<CookingJob> values_;
};
} // namespace elysium::food
