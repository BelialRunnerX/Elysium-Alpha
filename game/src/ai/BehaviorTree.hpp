// Intended function: Represent compact deterministic behavior-tree runtime nodes, cursors, cooldowns, and stable blackboard keys.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ai {
struct BehaviorNodeState {
    std::uint64_t actorId{};
    std::uint64_t treeId{};
    std::uint64_t nodeId{};
    std::uint64_t status{};
    std::uint64_t cursor{};
    std::uint64_t cooldown{};
};
class BehaviorNodeStateTable {
public:
 bool set(BehaviorNodeState value); bool remove(std::uint64_t id);
 [[nodiscard]] const BehaviorNodeState* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<BehaviorNodeState> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const BehaviorNodeState& value) noexcept; std::vector<BehaviorNodeState> rows_;
};
}
