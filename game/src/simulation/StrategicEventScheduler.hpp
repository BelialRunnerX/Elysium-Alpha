#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Schedule deterministic future events keyed by stable identities and simulation time without wall-clock dependence.
struct StrategicEventSchedulerRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct StrategicEventSchedulerState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class StrategicEventSchedulerSystem {
public:
    bool apply(const StrategicEventSchedulerRequest& request);
    bool erase(std::uint64_t targetId);
    const StrategicEventSchedulerState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, StrategicEventSchedulerState> states_;
};

} // namespace elysium
