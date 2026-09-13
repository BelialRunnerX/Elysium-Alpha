#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Schedule multi-ship convoy departures, escorts, rendezvous, and arrival windows using stable route summaries.
struct ConvoySchedulerRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct ConvoySchedulerState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class ConvoySchedulerSystem {
public:
    bool apply(const ConvoySchedulerRequest& request);
    bool erase(std::uint64_t targetId);
    const ConvoySchedulerState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, ConvoySchedulerState> states_;
};

} // namespace elysium
