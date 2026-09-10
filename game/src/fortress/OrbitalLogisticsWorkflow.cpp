#include "fortress/OrbitalLogisticsWorkflow.hpp"

namespace elysium::fortress {

WorkflowPlan dispatchFreight(const FreightManifest& manifest,
                             ShipState ship,
                             const RouteLeg& leg,
                             float gravity,
                             float cargoCapacity,
                             std::uint64_t tick,
                             std::uint32_t producer) {
    WorkflowPlan plan{};
    const float loadFraction = cargoCapacity > 0.0f ? manifest.mass / cargoCapacity : 2.0f;
    const float takeoff = shipTakeoffMargin(ship, gravity, loadFraction, 0.0f);
    if (takeoff < 0.0f) {
        plan.diagnostics.push_back(WorkflowDiagnostic{
            "takeoff_margin", "Ship cannot safely lift current freight manifest",
            PriorityBand::High, ship.id});
        return plan;
    }
    if (!canWarp(ship, leg.distanceLy, leg.fuelCost)) {
        plan.diagnostics.push_back(WorkflowDiagnostic{
            "warp_unreachable", "Route exceeds jump range or available fuel",
            PriorityBand::High, ship.id});
        return plan;
    }
    ship.destinationSystem = leg.toSystem;
    plan.commands.push(workflowHeader(tick, producer, 0), DispatchShipCommand{ship.id, leg.toSystem});
    plan.events.push_back(workflowEvent(FortressEventKind::ItemMoved,
                                        TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                        manifest.id, manifest.origin,
                                        "Inter-site freight manifest dispatched", manifest.mass));
    return plan;
}

WorkflowPlan dispatchSurfaceFreight(const FreightManifest& manifest,
                                    const VehicleState& vehicle,
                                    ContentId route,
                                    std::uint64_t tick,
                                    std::uint32_t producer) {
    WorkflowPlan plan{};
    if (manifest.mass > vehicle.cargoCapacity) {
        plan.diagnostics.push_back(WorkflowDiagnostic{
            "vehicle_capacity", "Freight manifest exceeds vehicle cargo capacity",
            PriorityBand::High, StableId{vehicle.id.value}});
        return plan;
    }
    plan.commands.push(workflowHeader(tick, producer, 0),
                       DispatchVehicleCommand{vehicle.id, manifest.destination, std::move(route)});
    plan.events.push_back(workflowEvent(FortressEventKind::ItemMoved,
                                        TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                        manifest.id, manifest.origin,
                                        "Surface freight manifest dispatched", manifest.mass));
    return plan;
}

} // namespace elysium::fortress
