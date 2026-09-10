#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Propagate bounded supply shortages and production disruptions between stable-ID markets without global inventory scans.
struct SupplyChainShockRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct SupplyChainShockState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class SupplyChainShockSystem {
public:
    bool apply(const SupplyChainShockRequest& request);
    bool erase(std::uint64_t targetId);
    const SupplyChainShockState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, SupplyChainShockState> states_;
};

} // namespace elysium
