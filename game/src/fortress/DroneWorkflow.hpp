#pragma once

#include "fortress/DroneSystems.hpp"
#include "fortress/WorkflowCommon.hpp"

#include <span>

namespace elysium::fortress {

struct DroneTaskRequest {
    StableId task{};
    DroneRole role{DroneRole::Hauling};
    float pathCost{};
    float hazard{};
    PriorityBand priority{PriorityBand::Normal};
};

StableId chooseDroneForTask(std::span<const DroneState> drones,
                            const DroneTaskRequest& task,
                            float signalQuality,
                            float weatherPenalty);
WorkflowPlan planDroneService(std::span<const DroneState> drones,
                              SiteId site,
                              std::uint64_t seed,
                              std::uint64_t tick,
                              std::uint32_t producer = 390);

} // namespace elysium::fortress
