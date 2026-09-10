#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Represent multi-faction federations, shared obligations, membership votes, and treaty-level authority.
struct FederationSystemRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct FederationSystemState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class FederationSystemSystem {
public:
    bool apply(const FederationSystemRequest& request);
    bool erase(std::uint64_t targetId);
    const FederationSystemState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, FederationSystemState> states_;
};

} // namespace elysium
