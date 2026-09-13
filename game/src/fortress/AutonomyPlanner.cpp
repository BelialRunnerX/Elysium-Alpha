#include "fortress/AutonomyPlanner.hpp"

namespace elysium::fortress {

std::optional<AutonomousChoice> chooseAutonomousJob(std::span<const JobCandidateView> candidates,
                                                    const JobScoreInput& workerTemplate) {
    std::optional<AutonomousChoice> best;
    for (const auto& candidate : candidates) {
        if (candidate.job == nullptr || candidate.job->state == JobState::Completed ||
            candidate.job->state == JobState::Cancelled || candidate.job->state == JobState::Failed) {
            continue;
        }
        auto worker = workerTemplate;
        worker.estimatedDistance = candidate.estimatedDistance;
        worker.hazard = candidate.hazard;
        const auto result = scoreJob(*candidate.job, worker);
        if (!result.eligible) continue;
        AutonomousChoice choice{candidate.job->id, result.score, result.reason};
        if (!best || choice.score > best->score ||
            (choice.score == best->score && choice.job.value < best->job.value)) {
            best = std::move(choice);
        }
    }
    return best;
}

} // namespace elysium::fortress
