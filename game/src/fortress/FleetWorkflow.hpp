#pragma once

#include "fortress/FleetSystems.hpp"
#include "fortress/WorkflowCommon.hpp"

namespace elysium::fortress {

WorkflowPlan planFleetInterception(const FleetState& attacker,
                                   const FleetState& defender,
                                   SiteId affectedSite,
                                   std::uint64_t seed,
                                   std::uint64_t epoch,
                                   std::uint64_t tick,
                                   std::uint32_t producer = 340);

} // namespace elysium::fortress
