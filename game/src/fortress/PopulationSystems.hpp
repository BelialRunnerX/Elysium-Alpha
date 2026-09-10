#pragma once

#include "fortress/Components.hpp"
#include "fortress/Persistence.hpp"

#include <vector>

namespace elysium::fortress {

struct HouseholdState {
    StableId id{};
    SiteId site{};
    std::vector<StableId> members;
    RoomId quarters{};
    float foodAccess{1.0f};
    float privacy{0.5f};
    float cohesion{0.5f};
};

struct PopulationCohort {
    StableId id{};
    SiteId site{};
    ContentId species;
    ContentId culture;
    ContentId professionFamily;
    float population{};
    float childrenFraction{};
    float elderFraction{};
    float health{1.0f};
    float skillLevel{};
    float loyalty{0.5f};
};

float householdWellbeing(const HouseholdState& household, float safety, float institutionAccess);
void advancePopulationCohort(PopulationCohort& cohort, float years, float foodSecurity,
                             float medicalAccess, float threatPressure);
PersistentIdentity promoteCohortMember(const PopulationCohort& cohort, std::uint64_t seed,
                                       std::uint64_t ordinal, std::string name);

} // namespace elysium::fortress
