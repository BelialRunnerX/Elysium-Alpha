#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Generate stable named galactic regions and strategic tags from the hierarchy seed without persistent bulk state.
struct RegionGeneratorRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct RegionGeneratorState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class RegionGeneratorSystem {
public:
    bool apply(const RegionGeneratorRequest& request);
    bool erase(std::uint64_t targetId);
    const RegionGeneratorState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, RegionGeneratorState> states_;
};

} // namespace elysium
