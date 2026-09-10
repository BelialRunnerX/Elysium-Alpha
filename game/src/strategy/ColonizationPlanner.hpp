#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Rank candidate worlds and sites for colonization using bounded strategic summaries and stable addresses.
struct ColonizationPlannerRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct ColonizationPlannerState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class ColonizationPlannerSystem {
public:
    bool apply(const ColonizationPlannerRequest& request);
    bool erase(std::uint64_t targetId);
    const ColonizationPlannerState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, ColonizationPlannerState> states_;
};

} // namespace elysium
