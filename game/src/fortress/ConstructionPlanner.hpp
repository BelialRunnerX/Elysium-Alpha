#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Expand blueprints into ordered material, excavation, support, utility, and finishing work packages.
struct ConstructionPlannerRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct ConstructionPlannerState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class ConstructionPlannerSystem {
public:
    bool apply(const ConstructionPlannerRequest& request);
    bool erase(std::uint64_t targetId);
    const ConstructionPlannerState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, ConstructionPlannerState> states_;
};

} // namespace elysium
