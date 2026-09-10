#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Build bounded neighbor links from generated systems and jump-range rules while preserving deterministic topology.
struct JumpNetworkRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct JumpNetworkState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class JumpNetworkSystem {
public:
    bool apply(const JumpNetworkRequest& request);
    bool erase(std::uint64_t targetId);
    const JumpNetworkState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, JumpNetworkState> states_;
};

} // namespace elysium
