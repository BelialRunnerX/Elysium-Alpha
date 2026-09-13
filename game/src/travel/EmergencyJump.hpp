#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Produce emergency escape jump candidates with deterministic risk, fuel, damage, and destination tradeoffs.
struct EmergencyJumpRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct EmergencyJumpState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class EmergencyJumpSystem {
public:
    bool apply(const EmergencyJumpRequest& request);
    bool erase(std::uint64_t targetId);
    const EmergencyJumpState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, EmergencyJumpState> states_;
};

} // namespace elysium
