// Intended function: Track embodied operative skill-use progression, diminishing returns, training bonuses, unlock thresholds, and milestones.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::gameplay {
struct OperativeSkill {
    std::uint64_t skillId{};
    std::uint64_t experience{};
    std::uint64_t rank{};
    std::uint64_t practiceToday{};
    double bonus{};
    std::uint64_t flags{};
};
class OperativeSkillStore {
public:
 bool put(OperativeSkill v); bool erase(std::uint64_t id);
 [[nodiscard]] const OperativeSkill* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<OperativeSkill>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const OperativeSkill& v) noexcept; std::vector<OperativeSkill> values_;
};
} // namespace elysium::gameplay
