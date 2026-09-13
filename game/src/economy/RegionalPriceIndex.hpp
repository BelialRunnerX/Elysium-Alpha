#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Aggregate bounded market observations into deterministic regional price indices for AI planning.
struct RegionalPriceIndexRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct RegionalPriceIndexState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class RegionalPriceIndexSystem {
public:
    bool apply(const RegionalPriceIndexRequest& request);
    bool erase(std::uint64_t targetId);
    const RegionalPriceIndexState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, RegionalPriceIndexState> states_;
};

} // namespace elysium
