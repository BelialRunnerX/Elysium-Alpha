#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Generate civilization-scale goals for expansion, defense, trade, research, migration, and diplomacy.
struct CivilizationPlannerRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct CivilizationPlannerState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class CivilizationPlannerSystem {
public:
    bool apply(const CivilizationPlannerRequest& request);
    bool erase(std::uint64_t targetId);
    const CivilizationPlannerState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, CivilizationPlannerState> states_;
};

} // namespace elysium
