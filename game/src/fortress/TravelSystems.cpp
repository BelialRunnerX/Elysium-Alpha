#include "fortress/Systems.hpp"

#include <algorithm>
#include <cmath>

namespace elysium::fortress {

float vehicleMobility(const VehicleState& vehicle, float terrainPenalty, float weatherPenalty) {
    if (vehicle.hull <= 0.0f || vehicle.power <= 0.0f) return 0.0f;
    const float cargo = vehicle.cargoCapacity > 0.0f ? saturate(vehicle.cargoMass / vehicle.cargoCapacity) : 1.0f;
    const float loadFactor = 1.0f - cargo * 0.35f;
    const float surface = 1.0f - saturate(terrainPenalty) * 0.55f;
    const float weather = 1.0f - saturate(weatherPenalty) * 0.45f;
    return std::max(0.0f, vehicle.mobility * loadFactor * surface * weather * saturate(vehicle.hull));
}

float shipTakeoffMargin(const ShipState& ship, float gravity, float cargoLoadFraction,
                        float weatherSeverity) {
    const float handling = std::max(0.1f, ship.atmosphericHandling);
    const float load = 1.0f + saturate(cargoLoadFraction) * 0.7f;
    const float environment = std::max(0.2f, gravity) * (1.0f + saturate(weatherSeverity) * 0.65f);
    return handling / (load * environment) - 0.75f;
}

bool canWarp(const ShipState& ship, float routeDistanceLy, float fuelCost) {
    return routeDistanceLy >= 0.0f && routeDistanceLy <= ship.jumpRangeLy && fuelCost >= 0.0f && ship.fuelCells >= fuelCost && ship.hull > 0.0f;
}

void advanceShipRoute(ShipState& ship, float routeDistanceLy, float lyPerHour, float hours) {
    if (routeDistanceLy <= 0.0f) {
        ship.routeProgress = 1.0f;
        return;
    }
    const float delta = std::max(0.0f, lyPerHour) * std::max(0.0f, hours) / routeDistanceLy;
    ship.routeProgress = saturate(ship.routeProgress + delta);
    if (ship.routeProgress >= 1.0f) {
        ship.currentSystem = ship.destinationSystem;
        ship.destinationSystem = 0;
    }
}

RemoteSiteDelta advanceRemoteSite(const SiteState& site, float days, float tradeThroughput,
                                  float threatPressure) {
    RemoteSiteDelta delta{};
    delta.site = site.id;
    const float d = std::max(0.0f, days);
    const float foodBalance = site.foodReserve > site.population * 0.5f ? 1.0f : -1.0f;
    const float threat = std::max(0.0f, threatPressure);
    delta.populationDelta = d * site.population * (0.0006f * foodBalance - threat * 0.0004f);
    delta.foodDelta = d * (site.industrialCapacity * 0.02f + tradeThroughput * 0.08f - site.population * 0.015f);
    delta.industryDelta = d * (site.prosperity * 0.004f + tradeThroughput * 0.002f - threat * 0.003f);
    delta.wealthDelta = d * (tradeThroughput * 0.12f + site.industrialCapacity * 0.04f - threat * 0.05f);
    delta.stabilityDelta = d * (foodBalance > 0.0f ? 0.0008f : -0.003f) - d * threat * 0.002f;
    return delta;
}

} // namespace elysium::fortress
