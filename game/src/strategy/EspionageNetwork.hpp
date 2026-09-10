#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Model intelligence assets, infiltration, discovery risk, and information freshness between factions.
struct EspionageNetworkRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct EspionageNetworkState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class EspionageNetworkSystem {
public:
    bool apply(const EspionageNetworkRequest& request);
    bool erase(std::uint64_t targetId);
    const EspionageNetworkState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, EspionageNetworkState> states_;
};

} // namespace elysium
