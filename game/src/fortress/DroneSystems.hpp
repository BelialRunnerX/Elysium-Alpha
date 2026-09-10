#pragma once

#include "fortress/Components.hpp"

namespace elysium::fortress {

enum class DroneRole : std::uint8_t { Hauling, Repair, Survey, Agriculture, Security, Medical };

struct DroneState {
    StableId id{};
    DroneRole role{DroneRole::Hauling};
    StableId port{};
    StableId assignedTarget{};
    JobId job{};
    float battery{1.0f};
    float cargo{};
    float cargoCapacity{8.0f};
    float condition{1.0f};
    float taskProgress{};
    bool returning{};
};

float droneTaskRate(const DroneState& drone, float pathCost, float signalQuality,
                    float hazard);
void advanceDrone(DroneState& drone, float dt, float pathCost, float signalQuality,
                  float hazard, bool atPort);

} // namespace elysium::fortress
