#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Track deterministic milestone facts and progress counters without owning platform achievement APIs.
struct AchievementTrackerRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct AchievementTrackerState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class AchievementTrackerSystem {
public:
    bool apply(const AchievementTrackerRequest& request);
    bool erase(std::uint64_t targetId);
    const AchievementTrackerState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, AchievementTrackerState> states_;
};

} // namespace elysium
