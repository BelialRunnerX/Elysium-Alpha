// Intended function: Choose citizen self-care, assigned work, emergency response, social activity, sleep, food, and recreation intentions.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ai {
struct CitizenDecisionState {
    std::uint64_t citizenId{};
    std::uint64_t decisionId{};
    std::uint64_t activityId{};
    double utility{};
    std::uint64_t durationTicks{};
    std::uint64_t flags{};
};
class CitizenDecisionStateTable {
public:
 bool set(CitizenDecisionState value); bool remove(std::uint64_t id);
 [[nodiscard]] const CitizenDecisionState* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<CitizenDecisionState> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const CitizenDecisionState& value) noexcept; std::vector<CitizenDecisionState> rows_;
};
}
