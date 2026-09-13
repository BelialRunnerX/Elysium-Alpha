#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Compact touched-chunk and stable-object journals while retaining tombstones and exactly-once semantics.
struct WorldDeltaCompactorRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct WorldDeltaCompactorState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class WorldDeltaCompactorSystem {
public:
    bool apply(const WorldDeltaCompactorRequest& request);
    bool erase(std::uint64_t targetId);
    const WorldDeltaCompactorState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, WorldDeltaCompactorState> states_;
};

} // namespace elysium
