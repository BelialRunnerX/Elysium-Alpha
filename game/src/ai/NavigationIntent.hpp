// Intended function: Represent actor movement goals, locomotion class, route policy, hazard tolerance, urgency, and cancellation token.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ai {
struct NavigationIntentState {
    std::uint64_t intentId{};
    std::uint64_t actorId{};
    std::uint64_t goalKey{};
    std::uint64_t locomotion{};
    std::uint64_t urgency{};
    std::uint64_t cancelToken{};
};
class NavigationIntentStateTable {
public:
 bool set(NavigationIntentState value); bool remove(std::uint64_t id);
 [[nodiscard]] const NavigationIntentState* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<NavigationIntentState> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const NavigationIntentState& value) noexcept; std::vector<NavigationIntentState> rows_;
};
}
