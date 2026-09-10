#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Assign citizens to deterministic work/rest shifts while respecting emergencies, professions, and fatigue.
struct ShiftSchedulerRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct ShiftSchedulerState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class ShiftSchedulerSystem {
public:
    bool apply(const ShiftSchedulerRequest& request);
    bool erase(std::uint64_t targetId);
    const ShiftSchedulerState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, ShiftSchedulerState> states_;
};

} // namespace elysium
