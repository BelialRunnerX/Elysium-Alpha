#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Build bounded multi-step action plans from high-level goals while leaving execution to authoritative systems.
struct HierarchicalPlannerRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct HierarchicalPlannerState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class HierarchicalPlannerSystem {
public:
    bool apply(const HierarchicalPlannerRequest& request);
    bool erase(std::uint64_t targetId);
    const HierarchicalPlannerState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, HierarchicalPlannerState> states_;
};

} // namespace elysium
