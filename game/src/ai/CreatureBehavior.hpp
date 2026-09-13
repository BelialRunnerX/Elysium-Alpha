// Intended function: Plan procedural creature feeding, nesting, territory, pack behavior, fleeing, ambush, curiosity, and migration.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ai {
struct CreatureDecision {
    std::uint64_t creatureId{};
    std::uint64_t behaviorId{};
    std::uint64_t targetId{};
    double energy{};
    double fear{};
    double aggression{};
};
class CreatureDecisionTable {
public:
 bool set(CreatureDecision value); bool remove(std::uint64_t id);
 [[nodiscard]] const CreatureDecision* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<CreatureDecision> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const CreatureDecision& value) noexcept; std::vector<CreatureDecision> rows_;
};
}
