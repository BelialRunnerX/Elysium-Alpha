#pragma once

#include "fortress/Components.hpp"
#include "fortress/CultureSystems.hpp"

#include <vector>

namespace elysium::fortress {

struct StrategicWorldSeed {
    std::uint64_t galaxySeed{};
    std::uint32_t generatorVersion{1};
};

struct StrategicWorldState {
    std::vector<CivilizationState> civilizations;
    std::vector<OrganizationState> organizations;
    std::vector<SiteState> sites;
    std::vector<HistoricalEvent> bootstrapHistory;
};

StrategicWorldState generateStrategicSeed(const StrategicWorldSeed& seed,
                                          std::span<const std::uint64_t> candidateSystems,
                                          std::uint32_t civilizationCount);

} // namespace elysium::fortress
