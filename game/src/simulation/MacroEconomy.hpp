#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Advance civilization production, consumption, reserves, infrastructure, and trade balances at strategic LOD.
struct MacroEconomyRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct MacroEconomyState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class MacroEconomySystem {
public:
    bool apply(const MacroEconomyRequest& request);
    bool erase(std::uint64_t targetId);
    const MacroEconomyState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, MacroEconomyState> states_;
};

} // namespace elysium
