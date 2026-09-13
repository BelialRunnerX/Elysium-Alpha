#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Propagate distress, trade, military, and discovery signals through jump topology with deterministic delay.
struct SignalPropagationRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct SignalPropagationState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class SignalPropagationSystem {
public:
    bool apply(const SignalPropagationRequest& request);
    bool erase(std::uint64_t targetId);
    const SignalPropagationState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, SignalPropagationState> states_;
};

} // namespace elysium
