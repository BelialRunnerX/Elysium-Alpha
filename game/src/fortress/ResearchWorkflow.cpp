#include "fortress/ResearchWorkflow.hpp"

namespace elysium::fortress {
namespace {
constexpr std::uint64_t kResearchJobLabel = 0x5245534541524348ULL;
}

WorkflowPlan planResearchCycle(ResearchProject project,
                               SiteId site,
                               std::span<const ResearcherContribution> researchers,
                               float sampleQuality,
                               float hours,
                               ContentId unlock,
                               std::uint64_t seed,
                               std::uint64_t tick,
                               std::uint32_t producer) {
    WorkflowPlan plan{};
    if (project.completed) return plan;
    std::uint32_t sequence = 0;
    if (researchers.empty()) {
        plan.diagnostics.push_back(WorkflowDiagnostic{
            "no_researcher", "Research project has no assigned researcher",
            PriorityBand::Normal, project.id});
        return plan;
    }
    for (std::size_t i = 0; i < researchers.size(); ++i) {
        const auto& contribution = researchers[i];
        advanceResearch(project, contribution.skill, contribution.focus,
                        contribution.facilityQuality, sampleQuality, hours);
        const auto jobId = makeDerivedId<JobId>(seed, contribution.researcher.value,
                                                kResearchJobLabel, tick + static_cast<std::uint64_t>(i));
        auto job = workflowJob(jobId, "elysium:job/research_project",
                               ContentId{"elysium:labor/research"}, PriorityBand::Normal,
                               contribution.researcher, SpatialAnchor{}, std::max(0.5f, hours));
        plan.commands.push(workflowHeader(tick, producer, sequence++), CreateJobCommand{job});
        if (project.completed) break;
    }
    if (project.completed) {
        plan.commands.push(workflowHeader(tick, producer, sequence++),
                           CompleteResearchCommand{project.id, std::move(unlock)});
        plan.events.push_back(workflowEvent(FortressEventKind::ResearchCompleted,
                                            TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                            project.id, site, "Research project completed"));
    }
    plan.commands.sortDeterministic();
    return plan;
}

} // namespace elysium::fortress
