#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Compute settlement wage bands from labor scarcity, skill demand, hazard pressure, and civic policy.
struct WagePolicyRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct WagePolicyState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class WagePolicySystem {
public:
    bool apply(const WagePolicyRequest& request);
    bool erase(std::uint64_t targetId);
    const WagePolicyState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, WagePolicyState> states_;
};

} // namespace elysium
