// Intended function: promote/demote ecology between cheap population tendencies and local simulated populations.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::wildlife {

struct PopulationTendency {
    std::uint64_t speciesKey{};
    double expectedPopulation{};
    double carryingCapacity{};
    double health01{1.0};
    double migrationPressure{};
};

struct LocalPopulationMember {
    std::uint64_t stableId{};
    std::uint64_t speciesKey{};
    double health01{1.0};
    bool historySignificant{};
};

struct EcologyPromotionResult {
    std::vector<LocalPopulationMember> members;
    double residualExpectedPopulation{};
};

class EcologyPromotion final {
public:
    [[nodiscard]] EcologyPromotionResult promote(
        std::uint64_t habitatSeed,
        PopulationTendency tendency,
        std::size_t localBudget) const;

    [[nodiscard]] PopulationTendency demote(
        PopulationTendency prior,
        const std::vector<LocalPopulationMember>& members,
        double unresolvedPopulation) const noexcept;
};

} // namespace elysium::wildlife
