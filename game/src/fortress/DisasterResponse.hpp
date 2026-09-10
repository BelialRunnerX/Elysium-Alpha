#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Coordinate emergency policies across alarms, evacuation, isolation, rescue, firefighting, and recovery.
struct DisasterResponseRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct DisasterResponseState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class DisasterResponseSystem {
public:
    bool apply(const DisasterResponseRequest& request);
    bool erase(std::uint64_t targetId);
    const DisasterResponseState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, DisasterResponseState> states_;
};

} // namespace elysium
