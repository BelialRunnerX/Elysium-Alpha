#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Generate maintenance work from machine condition, criticality, spare availability, and failure probability.
struct MaintenancePlannerRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct MaintenancePlannerState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class MaintenancePlannerSystem {
public:
    bool apply(const MaintenancePlannerRequest& request);
    bool erase(std::uint64_t targetId);
    const MaintenancePlannerState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, MaintenancePlannerState> states_;
};

} // namespace elysium
