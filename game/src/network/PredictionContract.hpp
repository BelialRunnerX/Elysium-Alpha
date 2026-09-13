#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Define reversible client-prediction envelopes for movement and direct-operative actions without server authority leakage.
struct PredictionContractRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct PredictionContractState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class PredictionContractSystem {
public:
    bool apply(const PredictionContractRequest& request);
    bool erase(std::uint64_t targetId);
    const PredictionContractState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, PredictionContractState> states_;
};

} // namespace elysium
