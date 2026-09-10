// Intended function: Track settlement/strategic policies for labor, rationing, trade, law, military posture, immigration, and emergency response.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::strategy {
struct PolicyState {
    std::uint64_t policyId{};
    std::uint64_t scopeId{};
    std::uint64_t category{};
    double value{};
    double priority{};
    std::uint64_t revision{};
};
class PolicyStateStore {
public:
 bool put(PolicyState v); bool erase(std::uint64_t id);
 [[nodiscard]] const PolicyState* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<PolicyState>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const PolicyState& v) noexcept; std::vector<PolicyState> values_;
};
} // namespace elysium::strategy
