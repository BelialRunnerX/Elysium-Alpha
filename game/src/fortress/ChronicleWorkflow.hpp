#pragma once

#include "fortress/WorkflowCommon.hpp"

#include <optional>
#include <unordered_map>

namespace elysium::fortress {

struct ChronicleQueryIndex {
    std::unordered_map<std::uint64_t, std::vector<HistoricalEventId>> byFigure;
    std::unordered_map<std::uint64_t, std::vector<HistoricalEventId>> bySite;
    std::unordered_map<std::uint64_t, std::vector<HistoricalEventId>> byArtifact;
    std::unordered_map<std::uint64_t, HistoricalEvent> events;
};

bool indexHistoricalEvent(ChronicleQueryIndex& index, const HistoricalEvent& event);
std::vector<HistoricalEventId> chronicleForFigure(const ChronicleQueryIndex& index, StableId figure);
std::vector<HistoricalEventId> chronicleForSite(const ChronicleQueryIndex& index, SiteId site);
std::vector<HistoricalEventId> chronicleForArtifact(const ChronicleQueryIndex& index, ArtifactId artifact);
WorkflowPlan recordChronicleEvent(const HistoricalEvent& event,
                                  const HistorySignificance& current,
                                  std::uint64_t tick,
                                  std::uint32_t producer = 180);

} // namespace elysium::fortress
