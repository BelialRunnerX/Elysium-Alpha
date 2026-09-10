#include "fortress/ChronicleWorkflow.hpp"

#include "fortress/Systems.hpp"

namespace elysium::fortress {

bool indexHistoricalEvent(ChronicleQueryIndex& index, const HistoricalEvent& event) {
    if (!event.id || index.events.contains(event.id.value)) return false;
    index.events.emplace(event.id.value, event);
    if (event.primary) index.byFigure[event.primary.value].push_back(event.id);
    if (event.secondary) index.byFigure[event.secondary.value].push_back(event.id);
    if (event.site) index.bySite[event.site.value].push_back(event.id);
    if (event.artifact) index.byArtifact[event.artifact.value].push_back(event.id);
    return true;
}

std::vector<HistoricalEventId> chronicleForFigure(const ChronicleQueryIndex& index, StableId figure) {
    const auto it = index.byFigure.find(figure.value);
    return it == index.byFigure.end() ? std::vector<HistoricalEventId>{} : it->second;
}

std::vector<HistoricalEventId> chronicleForSite(const ChronicleQueryIndex& index, SiteId site) {
    const auto it = index.bySite.find(site.value);
    return it == index.bySite.end() ? std::vector<HistoricalEventId>{} : it->second;
}

std::vector<HistoricalEventId> chronicleForArtifact(const ChronicleQueryIndex& index, ArtifactId artifact) {
    const auto it = index.byArtifact.find(artifact.value);
    return it == index.byArtifact.end() ? std::vector<HistoricalEventId>{} : it->second;
}

WorkflowPlan recordChronicleEvent(const HistoricalEvent& event,
                                  const HistorySignificance& current,
                                  std::uint64_t tick,
                                  std::uint32_t producer) {
    WorkflowPlan plan{};
    plan.commands.push(workflowHeader(tick, producer, 0), RecordHistoricalEventCommand{event});
    if (event.primary && shouldPromoteHistoricalFigure(current, event)) {
        plan.commands.push(workflowHeader(tick, producer, 1),
                           PromoteHistoricalFigureCommand{event.primary, historicalSignificance(event)});
    }
    plan.events.push_back(workflowEvent(FortressEventKind::HistoricalEventRecorded,
                                        event.time, event.primary, event.site, event.summary,
                                        event.significance));
    plan.commands.sortDeterministic();
    return plan;
}

} // namespace elysium::fortress
