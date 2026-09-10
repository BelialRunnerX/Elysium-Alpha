// Intended function: Allocate bounded per-tick budgets across AI, jobs, logistics, environment, history, saves, and presentation staging.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::simulation {
struct BudgetSlice {
    std::uint64_t sliceId{};
    std::uint64_t systemId{};
    double budget{};
    double consumed{};
    std::uint64_t priority{};
    std::uint64_t tick{};
};
class BudgetSliceTable {
public:
 bool set(BudgetSlice value); bool remove(std::uint64_t id);
 [[nodiscard]] const BudgetSlice* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<BudgetSlice> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const BudgetSlice& value) noexcept; std::vector<BudgetSlice> rows_;
};
}
