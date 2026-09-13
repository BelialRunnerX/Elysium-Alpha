#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Build deterministic stable-ID replication deltas from authoritative snapshots without exposing transient ECS IDs.
struct ReplicationDeltaRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct ReplicationDeltaState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class ReplicationDeltaSystem {
public:
    bool apply(const ReplicationDeltaRequest& request);
    bool erase(std::uint64_t targetId);
    const ReplicationDeltaState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, ReplicationDeltaState> states_;
};

} // namespace elysium
