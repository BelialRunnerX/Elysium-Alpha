#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Generate price concessions and negotiation stances from market data, standing, urgency, and personality.
struct TradeNegotiationAIRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct TradeNegotiationAIState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class TradeNegotiationAISystem {
public:
    bool apply(const TradeNegotiationAIRequest& request);
    bool erase(std::uint64_t targetId);
    const TradeNegotiationAIState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, TradeNegotiationAIState> states_;
};

} // namespace elysium
