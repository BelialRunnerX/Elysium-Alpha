// Intended function: Track social/psychological needs, satisfaction, stress contribution, institutions, relationships, and recovery.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::fortress {
struct SocialNeedState {
    std::uint64_t citizenId{};
    std::uint64_t needId{};
    double value{};
    double target{};
    double stressWeight{};
    std::uint64_t flags{};
};
class SocialNeedStateStore {
public:
 bool put(SocialNeedState v); bool erase(std::uint64_t id);
 [[nodiscard]] const SocialNeedState* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<SocialNeedState>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const SocialNeedState& v) noexcept; std::vector<SocialNeedState> values_;
};
} // namespace elysium::fortress
