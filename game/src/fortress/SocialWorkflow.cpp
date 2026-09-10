#include "fortress/SocialWorkflow.hpp"

namespace elysium::fortress {

WorkflowPlan applySocialEvent(RelationshipEdge relationship,
                              const Thought& thought,
                              const Personality& personality,
                              SiteId site,
                              std::uint64_t tick,
                              std::uint32_t producer) {
    WorkflowPlan plan{};
    const float delta = relationshipInteractionDelta(relationship, thought, personality);
    plan.commands.push(workflowHeader(tick, producer, 0),
                       AdjustRelationshipCommand{relationship.source, relationship.target,
                                                 delta, 0.5f * delta, 0.35f * delta,
                                                 delta < 0.0f ? -delta * 0.2f : 0.0f,
                                                 delta < 0.0f ? -delta * 0.5f : 0.0f});
    plan.commands.push(workflowHeader(tick, producer, 1), ApplyThoughtCommand{relationship.source, thought});
    plan.events.push_back(workflowEvent(FortressEventKind::RelationshipChanged,
                                        TimeStamp{static_cast<std::int64_t>(tick), 0, 0},
                                        relationship.source, site, "Social interaction changed persistent relationship",
                                        delta));
    plan.commands.sortDeterministic();
    return plan;
}

WorkflowPlan applyGrief(StableId citizen,
                        StableId deceased,
                        float relationshipAffinity,
                        float relationshipFamiliarity,
                        SiteId site,
                        std::uint64_t tick,
                        std::uint32_t producer) {
    WorkflowPlan plan{};
    const float magnitude = saturate(std::max(0.0f, relationshipAffinity) *
                                     (0.4f + 0.6f * saturate(relationshipFamiliarity)));
    Memory memory{};
    memory.key = "elysium:memory/grief_death";
    memory.valence = -magnitude;
    memory.strength = magnitude;
    memory.trauma = 0.5f * magnitude;
    memory.occurred = TimeStamp{static_cast<std::int64_t>(tick), 0, 0};
    memory.subject = deceased;
    plan.commands.push(workflowHeader(tick, producer, 0), ApplyMemoryCommand{citizen, memory});
    plan.commands.push(workflowHeader(tick, producer, 1),
                       AdjustStressCommand{citizen, 0.5f * magnitude, "grief after death"});
    plan.events.push_back(workflowEvent(FortressEventKind::CitizenDied, memory.occurred,
                                        deceased, site, "Death propagated grief to relationship network", magnitude));
    plan.commands.sortDeterministic();
    return plan;
}

} // namespace elysium::fortress
