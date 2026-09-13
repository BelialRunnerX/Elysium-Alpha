#pragma once

#include "fortress/Systems.hpp"

#include <optional>
#include <span>

namespace elysium::fortress {

struct JobCandidateView {
    const JobComponent* job{};
    float estimatedDistance{};
    float hazard{};
};

struct AutonomousChoice {
    JobId job{};
    float score{};
    std::string reason;
};

std::optional<AutonomousChoice> chooseAutonomousJob(std::span<const JobCandidateView> candidates,
                                                    const JobScoreInput& workerTemplate);

} // namespace elysium::fortress
