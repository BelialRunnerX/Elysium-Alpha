#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Plan local orbital transfers between surface, station, moon, and ship targets without owning flight physics.
struct OrbitalTransferPlannerRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct OrbitalTransferPlannerState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class OrbitalTransferPlannerSystem {
public:
    bool apply(const OrbitalTransferPlannerRequest& request);
    bool erase(std::uint64_t targetId);
    const OrbitalTransferPlannerState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, OrbitalTransferPlannerState> states_;
};

} // namespace elysium
