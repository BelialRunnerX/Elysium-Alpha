#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Manage temporary or persistent event-site anchors such as crashes, camps, battles, storms, and anomalies.
struct WorldEventSitesRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct WorldEventSitesState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class WorldEventSitesSystem {
public:
    bool apply(const WorldEventSitesRequest& request);
    bool erase(std::uint64_t targetId);
    const WorldEventSitesState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, WorldEventSitesState> states_;
};

} // namespace elysium
