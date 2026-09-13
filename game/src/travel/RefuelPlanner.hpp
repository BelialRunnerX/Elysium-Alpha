#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Plan bounded refueling stops from ship fuel state, route topology, station availability, and reserve policy.
struct RefuelPlannerRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct RefuelPlannerState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class RefuelPlannerSystem {
public:
    bool apply(const RefuelPlannerRequest& request);
    bool erase(std::uint64_t targetId);
    const RefuelPlannerState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, RefuelPlannerState> states_;
};

} // namespace elysium
