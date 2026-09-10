#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Track persistent depletion and regeneration of renewable and strategic resource nodes by stable address.
struct ResourceDepletionRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct ResourceDepletionState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class ResourceDepletionSystem {
public:
    bool apply(const ResourceDepletionRequest& request);
    bool erase(std::uint64_t targetId);
    const ResourceDepletionState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, ResourceDepletionState> states_;
};

} // namespace elysium
