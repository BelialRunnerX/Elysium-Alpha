#include "fortress/HouseholdWorkflow.hpp"

namespace elysium::fortress {

WorkflowPlan planHouseholdNeeds(const HouseholdPressure& pressure,
                                std::uint64_t seed,
                                std::uint64_t tick,
                                std::uint32_t producer) {
    WorkflowPlan plan{};
    const float wellbeing = householdWellbeing(pressure.household, pressure.safety,
                                                pressure.institutionAccess);
    std::uint32_t sequence = 0;
    if (pressure.housingCapacity < 0.75f || !pressure.household.quarters) {
        const auto id = makeDerivedId<JobId>(seed, pressure.household.id.value,
                                             0x484F555345484F4CULL, tick);
        auto housing = workflowJob(id, "elysium:job/assign_or_build_quarters",
                                   ContentId{"elysium:labor/furniture_install"}, PriorityBand::High,
                                   pressure.household.id, SpatialAnchor{}, 1.0f);
        plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{housing});
    }
    if (pressure.household.foodAccess < 0.4f) {
        const auto id = makeDerivedId<JobId>(seed, pressure.household.id.value ^ 0x464F4F44ULL,
                                             0x484F555345484F4CULL, tick);
        auto food = workflowJob(id, "elysium:job/household_food_supply",
                                ContentId{"elysium:labor/haul_food"}, PriorityBand::Urgent,
                                pressure.household.id, SpatialAnchor{}, 0.75f);
        plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{food});
    }
    if (wellbeing < 0.45f) {
        for (const auto member : pressure.household.members) {
            plan.commands.push(workflowHeader(tick, producer, sequence++),
                               AdjustStressCommand{member, 0.05f, "household wellbeing is poor"});
        }
    }
    plan.commands.sortDeterministic();
    return plan;
}

} // namespace elysium::fortress
