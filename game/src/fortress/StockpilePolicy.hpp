// Intended function: Represent fortress stockpile filters, priorities, capacity targets, hazard rules, and give/take links.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::fortress {
struct StockpilePolicyState {
    std::uint64_t stockpileId{};
    std::uint64_t filterHash{};
    double priority{};
    double capacity{};
    std::uint64_t reserved{};
    std::uint64_t flags{};
};
class StockpilePolicyStateStore {
public:
 bool put(StockpilePolicyState v); bool erase(std::uint64_t id);
 [[nodiscard]] const StockpilePolicyState* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<StockpilePolicyState>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const StockpilePolicyState& v) noexcept; std::vector<StockpilePolicyState> values_;
};
} // namespace elysium::fortress
