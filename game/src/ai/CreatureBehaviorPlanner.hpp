#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Generate territorial, hunting, fleeing, nesting, social, and curiosity behaviors for wildlife actors.
struct CreatureBehaviorPlannerRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct CreatureBehaviorPlannerState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class CreatureBehaviorPlannerSystem {
public:
    bool apply(const CreatureBehaviorPlannerRequest& request);
    bool erase(std::uint64_t targetId);
    const CreatureBehaviorPlannerState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, CreatureBehaviorPlannerState> states_;
};

} // namespace elysium
