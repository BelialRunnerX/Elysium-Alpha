#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Select stable entities, chunks, and events relevant to a future client connection using bounded spatial interest.
struct InterestManagementRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct InterestManagementState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class InterestManagementSystem {
public:
    bool apply(const InterestManagementRequest& request);
    bool erase(std::uint64_t targetId);
    const InterestManagementState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, InterestManagementState> states_;
};

} // namespace elysium
