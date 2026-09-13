// Intended function: Resolve contextual interactions with voxels, machines, actors, items, vehicles, terminals, doors, and dialogue targets.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::gameplay {
struct InteractionTarget {
    double targetId{};
    std::uint64_t kind{};
    double distance{};
    double priority{};
    std::uint64_t actionMask{};
    std::uint64_t flags{};
};
class InteractionTargetStore {
public:
 bool put(InteractionTarget v); bool erase(std::uint64_t id);
 [[nodiscard]] const InteractionTarget* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<InteractionTarget>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const InteractionTarget& v) noexcept; std::vector<InteractionTarget> values_;
};
} // namespace elysium::gameplay
