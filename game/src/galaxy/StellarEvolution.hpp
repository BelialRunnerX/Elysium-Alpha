#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Advance stars through deterministic long-horizon stages that influence radiation, habitable zones, and hazards.
struct StellarEvolutionRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct StellarEvolutionState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class StellarEvolutionSystem {
public:
    bool apply(const StellarEvolutionRequest& request);
    bool erase(std::uint64_t targetId);
    const StellarEvolutionState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, StellarEvolutionState> states_;
};

} // namespace elysium
