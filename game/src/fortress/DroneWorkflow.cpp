#include "fortress/DroneWorkflow.hpp"

namespace elysium::fortress {

StableId chooseDroneForTask(std::span<const DroneState> drones,
                            const DroneTaskRequest& task,
                            float signalQuality,
                            float weatherPenalty) {
    StableId best{};
    float bestRate = -1.0f;
    for (const auto& drone : drones) {
        if (drone.role != task.role || drone.condition <= 0.05f || drone.battery <= 0.05f) continue;
        const float rate = droneTaskRate(drone, task.pathCost, signalQuality,
                                         task.hazard + weatherPenalty);
        if (!best || rate > bestRate || (rate == bestRate && drone.id.value < best.value)) {
            best = drone.id;
            bestRate = rate;
        }
    }
    return best;
}

WorkflowPlan planDroneService(std::span<const DroneState> drones,
                              SiteId site,
                              std::uint64_t seed,
                              std::uint64_t tick,
                              std::uint32_t producer) {
    WorkflowPlan plan{};
    std::uint32_t sequence = 0;
    for (std::size_t i = 0; i < drones.size(); ++i) {
        const auto& drone = drones[i];
        if (drone.condition >= 0.45f && drone.battery >= 0.15f) continue;
        const auto id = makeDerivedId<JobId>(seed, drone.id.value,
                                             0x44524F4E45534552ULL,
                                             tick + static_cast<std::uint64_t>(i));
        auto service = workflowJob(id, "elysium:job/service_drone",
                                   ContentId{"elysium:labor/drone_operation"}, PriorityBand::High,
                                   drone.id, SpatialAnchor{}, 0.75f);
        plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{service});
    }
    if (!plan.commands.empty()) {
        plan.events.push_back(workflowEvent(FortressEventKind::JobCreated,
                                            TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                            {}, site, "Drone maintenance/charging jobs generated"));
    }
    plan.commands.sortDeterministic();
    return plan;
}

} // namespace elysium::fortress
