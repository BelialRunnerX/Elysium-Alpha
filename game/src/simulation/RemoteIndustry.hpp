#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Advance off-shard industry from aggregate inputs, capacities, outages, workforce, and logistics access.
struct RemoteIndustryRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct RemoteIndustryState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class RemoteIndustrySystem {
public:
    bool apply(const RemoteIndustryRequest& request);
    bool erase(std::uint64_t targetId);
    const RemoteIndustryState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, RemoteIndustryState> states_;
};

} // namespace elysium
