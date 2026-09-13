#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Record first-discovery facts for worlds, species, ruins, technologies, anomalies, and artifacts.
struct DiscoveryHistoryRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct DiscoveryHistoryState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class DiscoveryHistorySystem {
public:
    bool apply(const DiscoveryHistoryRequest& request);
    bool erase(std::uint64_t targetId);
    const DiscoveryHistoryState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, DiscoveryHistoryState> states_;
};

} // namespace elysium
