#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Estimate contested-border pressure from claims, fleets, settlements, hazards, and diplomatic standing.
struct BorderPressureRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct BorderPressureState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class BorderPressureSystem {
public:
    bool apply(const BorderPressureRequest& request);
    bool erase(std::uint64_t targetId);
    const BorderPressureState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, BorderPressureState> states_;
};

} // namespace elysium
