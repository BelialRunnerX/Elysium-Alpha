// Intended function: Derive citizen professions from skills, assigned work, social role, institutions, and historical identity.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::fortress {
struct ProfessionState {
    std::uint64_t citizenId{};
    std::uint64_t professionId{};
    std::uint64_t rank{};
    std::uint64_t prestige{};
    std::uint64_t assignmentHash{};
    std::uint64_t flags{};
};
class ProfessionStateStore {
public:
 bool put(ProfessionState v); bool erase(std::uint64_t id);
 [[nodiscard]] const ProfessionState* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<ProfessionState>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const ProfessionState& v) noexcept; std::vector<ProfessionState> values_;
};
} // namespace elysium::fortress
