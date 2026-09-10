#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Generate cover, focus-fire, flank, retreat, breach, and defend intents from local tactical summaries.
struct SquadTacticsPlannerRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct SquadTacticsPlannerState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class SquadTacticsPlannerSystem {
public:
    bool apply(const SquadTacticsPlannerRequest& request);
    bool erase(std::uint64_t targetId);
    const SquadTacticsPlannerState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, SquadTacticsPlannerState> states_;
};

} // namespace elysium
