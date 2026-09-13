#pragma once

#include "fortress/WorkflowCommon.hpp"
#include "fortress/Systems.hpp"

#include <span>

namespace elysium::fortress {

struct MachineMaintenanceView {
    MachineState machine{};
    MaintenanceState maintenance{};
    SpatialAnchor location{};
};

WorkflowPlan planMaintenance(std::span<const MachineMaintenanceView> machines,
                             SiteId site,
                             std::uint64_t seed,
                             std::uint64_t tick,
                             std::uint32_t producer = 380);

} // namespace elysium::fortress
