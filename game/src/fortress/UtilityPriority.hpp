#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Allocate scarce power, atmosphere, coolant, and data bandwidth according to configurable priority classes.
struct UtilityPriorityRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct UtilityPriorityState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class UtilityPrioritySystem {
public:
    bool apply(const UtilityPriorityRequest& request);
    bool erase(std::uint64_t targetId);
    const UtilityPriorityState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, UtilityPriorityState> states_;
};

} // namespace elysium
