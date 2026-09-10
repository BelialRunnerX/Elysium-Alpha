// Intended function: Represent animation graph state, locomotion, action montage, hit reaction, equipment pose, facial state, and blend parameters.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::render {
struct AnimationGraphState {
    std::uint64_t stableId{};
    std::uint64_t graphId{};
    std::uint64_t stateId{};
    double time{};
    double blend{};
    std::uint64_t flags{};
};
class AnimationGraphStateCollection {
public:
 bool store(AnimationGraphState value); bool erase(std::uint64_t id); [[nodiscard]] const AnimationGraphState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<AnimationGraphState>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const AnimationGraphState& v) noexcept; std::vector<AnimationGraphState> rows_;
};
}
