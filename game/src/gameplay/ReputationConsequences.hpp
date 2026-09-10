#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Translate player actions into bounded faction, settlement, profession, and individual reputation deltas.
struct ReputationConsequencesRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct ReputationConsequencesState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class ReputationConsequencesSystem {
public:
    bool apply(const ReputationConsequencesRequest& request);
    bool erase(std::uint64_t targetId);
    const ReputationConsequencesState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, ReputationConsequencesState> states_;
};

} // namespace elysium
