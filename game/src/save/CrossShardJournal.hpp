#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Record durable cross-shard references, transfers, and dependency closure for transactional publication.
struct CrossShardJournalRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct CrossShardJournalState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class CrossShardJournalSystem {
public:
    bool apply(const CrossShardJournalRequest& request);
    bool erase(std::uint64_t targetId);
    const CrossShardJournalState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, CrossShardJournalState> states_;
};

} // namespace elysium
