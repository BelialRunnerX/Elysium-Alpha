#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Advance biome transitions from climate, disturbance, contamination, terraforming, and ecological pressure.
struct BiomeSuccessionRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct BiomeSuccessionState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class BiomeSuccessionSystem {
public:
    bool apply(const BiomeSuccessionRequest& request);
    bool erase(std::uint64_t targetId);
    const BiomeSuccessionState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, BiomeSuccessionState> states_;
};

} // namespace elysium
