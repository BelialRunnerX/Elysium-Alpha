#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Score rooms from space, privacy, furnishing, atmosphere, noise, beauty, contamination, and ownership.
struct RoomQualitySystemRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct RoomQualitySystemState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class RoomQualitySystemSystem {
public:
    bool apply(const RoomQualitySystemRequest& request);
    bool erase(std::uint64_t targetId);
    const RoomQualitySystemState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, RoomQualitySystemState> states_;
};

} // namespace elysium
