#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Create bounded recovery checkpoints for active shards without becoming a second authoritative save format.
struct RecoverySnapshotRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct RecoverySnapshotState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class RecoverySnapshotSystem {
public:
    bool apply(const RecoverySnapshotRequest& request);
    bool erase(std::uint64_t targetId);
    const RecoverySnapshotState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, RecoverySnapshotState> states_;
};

} // namespace elysium
