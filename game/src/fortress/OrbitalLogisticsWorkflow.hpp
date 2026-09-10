#pragma once

#include "fortress/WorkflowCommon.hpp"
#include "fortress/Systems.hpp"

#include <span>

namespace elysium::fortress {

struct FreightManifest {
    StableId id{};
    SiteId origin{};
    SiteId destination{};
    std::vector<std::pair<ContentId, float>> cargo;
    float mass{};
    float priority{};
    bool inspected{};
};

struct RouteLeg {
    std::uint64_t fromSystem{};
    std::uint64_t toSystem{};
    float distanceLy{};
    float fuelCost{};
    float hazard{};
};

WorkflowPlan dispatchFreight(const FreightManifest& manifest,
                             ShipState ship,
                             const RouteLeg& leg,
                             float gravity,
                             float cargoCapacity,
                             std::uint64_t tick,
                             std::uint32_t producer = 190);
WorkflowPlan dispatchSurfaceFreight(const FreightManifest& manifest,
                                    const VehicleState& vehicle,
                                    ContentId route,
                                    std::uint64_t tick,
                                    std::uint32_t producer = 191);

} // namespace elysium::fortress
