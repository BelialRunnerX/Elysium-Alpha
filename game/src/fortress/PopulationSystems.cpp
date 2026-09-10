#include "fortress/PopulationSystems.hpp"

#include <algorithm>
#include <cmath>

namespace elysium::fortress {

float householdWellbeing(const HouseholdState& household, float safety, float institutionAccess) {
    return saturate(household.foodAccess * 0.30f + household.privacy * 0.15f + household.cohesion * 0.25f +
                    saturate(safety) * 0.20f + saturate(institutionAccess) * 0.10f);
}

void advancePopulationCohort(PopulationCohort& cohort, float years, float foodSecurity,
                             float medicalAccess, float threatPressure) {
    const float y = std::max(0.0f, years);
    const float food = saturate(foodSecurity);
    const float medicine = saturate(medicalAccess);
    const float threat = saturate(threatPressure);
    const float birth = 0.018f * food * (1.0f - threat * 0.6f);
    const float death = 0.008f * (1.2f - medicine * 0.6f) + threat * 0.025f + (1.0f - food) * 0.02f;
    cohort.population = std::max(0.0f, cohort.population * std::exp((birth - death) * y));
    cohort.childrenFraction = saturate(cohort.childrenFraction + y * (birth * 0.5f - 0.01f));
    cohort.elderFraction = saturate(cohort.elderFraction + y * (0.006f - death * 0.18f));
    cohort.health = saturate(cohort.health + y * (medicine * 0.02f + food * 0.01f - threat * 0.04f));
    cohort.skillLevel = std::max(0.0f, cohort.skillLevel + y * 0.04f * cohort.health);
    cohort.loyalty = saturate(cohort.loyalty + y * 0.01f * food - y * threat * 0.02f);
}

PersistentIdentity promoteCohortMember(const PopulationCohort& cohort, std::uint64_t seed,
                                       std::uint64_t ordinal, std::string name) {
    PersistentIdentity identity{};
    identity.stableId = makeDerivedId<StableId>(seed, cohort.id.value, 0x434F484F52545052ULL, ordinal);
    identity.name = std::move(name);
    identity.shard = SimulationShard::ActiveFortress;
    identity.schemaVersion = 1;
    return identity;
}

} // namespace elysium::fortress
