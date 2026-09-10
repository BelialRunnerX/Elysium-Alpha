#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Advance long-timescale ecological, agricultural, economic, and migration changes on bounded seasonal cadence.
struct SeasonalTickRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct SeasonalTickState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class SeasonalTickSystem {
public:
    bool apply(const SeasonalTickRequest& request);
    bool erase(std::uint64_t targetId);
    const SeasonalTickState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, SeasonalTickState> states_;
};

} // namespace elysium
