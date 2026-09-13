#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Simulate remote populations as cohorts with births, deaths, migration, labor, morale, and skill summaries.
struct PopulationAggregateRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct PopulationAggregateState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class PopulationAggregateSystem {
public:
    bool apply(const PopulationAggregateRequest& request);
    bool erase(std::uint64_t targetId);
    const PopulationAggregateState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, PopulationAggregateState> states_;
};

} // namespace elysium
