// Intended function: Select cover, range bands, flanks, abilities, retreats, focus targets, and squad-role actions from tactical state.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ai {
struct TacticalDecision {
    std::uint64_t decisionId{};
    std::uint64_t actorId{};
    std::uint64_t targetId{};
    std::uint64_t actionId{};
    double score{};
    std::uint64_t expiresTick{};
};
class TacticalDecisionTable {
public:
 bool set(TacticalDecision value); bool remove(std::uint64_t id);
 [[nodiscard]] const TacticalDecision* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<TacticalDecision> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const TacticalDecision& value) noexcept; std::vector<TacticalDecision> rows_;
};
}
