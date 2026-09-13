#include "fortress/FleetSystems.hpp"

#include <algorithm>

namespace elysium::fortress {

FleetBattleResult resolveFleetBattle(const FleetState& attacker, const FleetState& defender,
                                     std::uint64_t seed, std::uint64_t epoch) {
    const auto token = deterministicToken(seed, attacker.id.value ^ defender.id.value, 0x464C454554424154ULL, epoch);
    const float variance = 0.85f + static_cast<float>(token & 0xFFFFU) / 65535.0f * 0.30f;
    const float attackPower = attacker.strength * (0.5f + saturate(attacker.readiness) * 0.5f) * variance;
    const float defensePower = defender.strength * (0.5f + saturate(defender.readiness) * 0.5f) * (2.0f - variance);
    const float total = std::max(0.1f, attackPower + defensePower);
    FleetBattleResult result{};
    result.attackerWon = attackPower >= defensePower;
    result.attackerLoss = saturate(defensePower / total * (result.attackerWon ? 0.6f : 1.0f));
    result.defenderLoss = saturate(attackPower / total * (result.attackerWon ? 1.0f : 0.6f));
    result.collateral = saturate((attackPower + defensePower) / 1000.0f) * 0.25f;
    return result;
}

void advanceFleet(FleetState& fleet, float routeDistance, float speed, float hours) {
    if (routeDistance <= 0.0f || fleet.destination == 0) return;
    const float delta = std::max(0.0f, speed) * std::max(0.0f, hours) / routeDistance;
    fleet.travelProgress = saturate(fleet.travelProgress + delta);
    fleet.fuel = std::max(0.0f, fleet.fuel - delta * 0.5f);
    if (fleet.travelProgress >= 1.0f) {
        fleet.system = fleet.destination;
        fleet.destination = 0;
        fleet.travelProgress = 0.0f;
    }
}

} // namespace elysium::fortress
