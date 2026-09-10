#pragma once

#include "fortress/Components.hpp"

#include <vector>

namespace elysium::fortress {

struct FleetState {
    StableId id{};
    OrganizationId owner{};
    StableId commander{};
    std::uint64_t system{};
    std::uint64_t destination{};
    float strength{};
    float readiness{};
    float fuel{};
    float cargo{};
    float travelProgress{};
    ContentId mission;
    std::vector<StableId> ships;
};

struct FleetBattleResult {
    float attackerLoss{};
    float defenderLoss{};
    bool attackerWon{};
    float collateral{};
};

FleetBattleResult resolveFleetBattle(const FleetState& attacker, const FleetState& defender,
                                     std::uint64_t seed, std::uint64_t epoch);
void advanceFleet(FleetState& fleet, float routeDistance, float speed, float hours);

} // namespace elysium::fortress
