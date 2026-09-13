#pragma once

#include "fortress/CourtSystems.hpp"
#include "fortress/FileKnowledgeSystems.hpp"
#include "fortress/WorkflowCommon.hpp"
#include "fortress/Systems.hpp"

namespace elysium::fortress {

struct EmpireObservation {
    FileFact fact{};
    float sensorQuality{1.0f};
    float concealment{};
};

void applyEmpireObservation(FileKnowledge& knowledge, EmpireObservation observation);
WorkflowPlan planRegisterPressure(const StandingState& standing,
                                  std::uint64_t system,
                                  SiteId site,
                                  float industrialActivity,
                                  float visibleDefenses,
                                  std::uint64_t seed,
                                  std::uint64_t tick,
                                  std::uint32_t producer = 270);

} // namespace elysium::fortress
