// Intended function: Track repeating/conditional work orders, quotas, input availability, target stock, and production priority.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::fortress {
struct WorkOrderState {
    std::uint64_t orderId{};
    std::uint64_t recipeId{};
    double targetCount{};
    std::uint64_t completedCount{};
    double priority{};
    std::uint64_t flags{};
};
class WorkOrderStateStore {
public:
 bool put(WorkOrderState v); bool erase(std::uint64_t id);
 [[nodiscard]] const WorkOrderState* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<WorkOrderState>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const WorkOrderState& v) noexcept; std::vector<WorkOrderState> values_;
};
} // namespace elysium::fortress
