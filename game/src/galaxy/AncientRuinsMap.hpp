#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Generate sparse civilization-era ruin markers and discovery state keyed by stable system/site identities.
struct AncientRuinsMapRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct AncientRuinsMapState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class AncientRuinsMapSystem {
public:
    bool apply(const AncientRuinsMapRequest& request);
    bool erase(std::uint64_t targetId);
    const AncientRuinsMapState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, AncientRuinsMapState> states_;
};

} // namespace elysium
