// Intended function: Publish immutable ECS presentation snapshots for animation, HUD, audio, effects, and renderer interpolation.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ecs {
struct PresentationSnapshot {
    std::uint64_t snapshotId{};
    std::uint64_t stableId{};
    std::uint64_t stateHash{};
    std::uint64_t animationId{};
    std::uint64_t effectMask{};
    std::uint64_t tick{};
};
class PresentationSnapshotCollection {
public:
 bool store(PresentationSnapshot value); bool erase(std::uint64_t id); [[nodiscard]] const PresentationSnapshot* find(std::uint64_t id) const; [[nodiscard]] const std::vector<PresentationSnapshot>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const PresentationSnapshot& v) noexcept; std::vector<PresentationSnapshot> rows_;
};
}
