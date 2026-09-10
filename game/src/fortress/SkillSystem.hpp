// Intended function: Track simulation skills, practice, learning rate, effective penalties, mentorship, and profession-facing proficiency.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::fortress {
struct SkillState {
    std::uint64_t citizenId{};
    std::uint64_t skillId{};
    std::uint64_t experience{};
    std::uint64_t rank{};
    std::uint64_t effectiveRank{};
    std::uint64_t flags{};
};
class SkillStateStore {
public:
 bool put(SkillState v); bool erase(std::uint64_t id);
 [[nodiscard]] const SkillState* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<SkillState>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const SkillState& v) noexcept; std::vector<SkillState> values_;
};
} // namespace elysium::fortress
