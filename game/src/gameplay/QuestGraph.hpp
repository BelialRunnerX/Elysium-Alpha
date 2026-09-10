#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Represent branching quest dependencies, alternate completion paths, failure transitions, and durable objective identities.
struct QuestGraphRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct QuestGraphState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class QuestGraphSystem {
public:
    bool apply(const QuestGraphRequest& request);
    bool erase(std::uint64_t targetId);
    const QuestGraphState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, QuestGraphState> states_;
};

} // namespace elysium
