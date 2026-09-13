#pragma once

#include "fortress/WorldEvents.hpp"
#include "fortress/WorkflowCommon.hpp"

namespace elysium::fortress {

WorkflowPlan planPlanetaryIncident(const PlanetaryEvent& event,
                                   std::uint64_t seed,
                                   std::uint64_t tick,
                                   std::uint32_t producer = 280);

} // namespace elysium::fortress
