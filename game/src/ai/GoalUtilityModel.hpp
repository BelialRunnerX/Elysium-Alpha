#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Score competing citizen, squad, creature, and strategic goals using deterministic weighted utility curves.
struct GoalUtilityModelRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct GoalUtilityModelState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class GoalUtilityModelSystem {
public:
    bool apply(const GoalUtilityModelRequest& request);
    bool erase(std::uint64_t targetId);
    const GoalUtilityModelState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, GoalUtilityModelState> states_;
};

} // namespace elysium
