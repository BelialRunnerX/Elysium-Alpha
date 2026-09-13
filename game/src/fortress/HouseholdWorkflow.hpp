#pragma once

#include "fortress/PopulationSystems.hpp"
#include "fortress/WorkflowCommon.hpp"

namespace elysium::fortress {

struct HouseholdPressure {
    HouseholdState household{};
    float safety{1.0f};
    float institutionAccess{1.0f};
    float housingCapacity{1.0f};
};

WorkflowPlan planHouseholdNeeds(const HouseholdPressure& pressure,
                                std::uint64_t seed,
                                std::uint64_t tick,
                                std::uint32_t producer = 370);

} // namespace elysium::fortress
