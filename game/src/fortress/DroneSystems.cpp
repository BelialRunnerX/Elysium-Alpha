#include "fortress/DroneSystems.hpp"

#include <algorithm>

namespace elysium::fortress {

float droneTaskRate(const DroneState& drone, float pathCost, float signalQuality,
                    float hazard) {
    if (drone.condition <= 0.05f || drone.battery <= 0.02f) return 0.0f;
    const float path = 1.0f / (1.0f + std::max(0.0f, pathCost) * 0.05f);
    const float signal = 0.35f + saturate(signalQuality) * 0.65f;
    const float safety = 1.0f - saturate(hazard) * (drone.role == DroneRole::Security ? 0.2f : 0.65f);
    const float role = drone.role == DroneRole::Repair ? 0.9f : drone.role == DroneRole::Hauling ? 1.0f : 0.8f;
    return std::max(0.0f, path * signal * safety * role * drone.condition);
}

void advanceDrone(DroneState& drone, float dt, float pathCost, float signalQuality,
                  float hazard, bool atPort) {
    const float step = std::max(0.0f, dt);
    if (atPort && (drone.returning || drone.battery < 0.95f)) {
        drone.battery = saturate(drone.battery + step * 0.12f);
        drone.returning = drone.battery < 0.98f;
        if (!drone.returning) drone.taskProgress = 0.0f;
        return;
    }
    const float rate = droneTaskRate(drone, pathCost, signalQuality, hazard);
    drone.taskProgress = saturate(drone.taskProgress + rate * step * 0.05f);
    drone.battery = saturate(drone.battery - step * (0.01f + rate * 0.012f));
    drone.condition = saturate(drone.condition - step * saturate(hazard) * 0.0004f);
    if (drone.battery < 0.15f || drone.taskProgress >= 1.0f) drone.returning = true;
}

} // namespace elysium::fortress
