#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Track bounded leadership succession, houses, inheritances, rivalries, and legitimacy across civilizations.
struct DynastyHistoryRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct DynastyHistoryState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class DynastyHistorySystem {
public:
    bool apply(const DynastyHistoryRequest& request);
    bool erase(std::uint64_t targetId);
    const DynastyHistoryState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, DynastyHistoryState> states_;
};

} // namespace elysium
