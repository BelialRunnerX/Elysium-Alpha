// Intended function: Coordinate squad formation, role assignment, focus targets, suppression, retreat, rally, breach, and evacuation behavior.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ai {
struct SquadDecision {
    std::uint64_t squadId{};
    std::uint64_t orderId{};
    std::uint64_t targetId{};
    std::uint64_t formationId{};
    double readiness{};
    std::uint64_t state{};
};
class SquadDecisionTable {
public:
 bool set(SquadDecision value); bool remove(std::uint64_t id);
 [[nodiscard]] const SquadDecision* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<SquadDecision> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const SquadDecision& value) noexcept; std::vector<SquadDecision> rows_;
};
}
