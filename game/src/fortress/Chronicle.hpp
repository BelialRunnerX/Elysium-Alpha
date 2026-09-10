#pragma once

#include "fortress/Components.hpp"

#include <unordered_map>
#include <vector>

namespace elysium::fortress {

struct ChronicleIndex {
    std::vector<HistoricalEvent> events;
    std::unordered_map<std::uint64_t, std::vector<std::size_t>> byFigure;
    std::unordered_map<std::uint64_t, std::vector<std::size_t>> bySite;
    std::unordered_map<std::uint64_t, std::vector<std::size_t>> byArtifact;
    std::unordered_map<std::uint64_t, std::vector<std::size_t>> byOrganization;
};

void rebuildChronicleIndex(ChronicleIndex& index);
void appendChronicleEvent(ChronicleIndex& index, HistoricalEvent event);
std::vector<const HistoricalEvent*> eventsForFigure(const ChronicleIndex& index, StableId figure);
std::vector<const HistoricalEvent*> eventsForSite(const ChronicleIndex& index, SiteId site);
std::vector<const HistoricalEvent*> eventsForArtifact(const ChronicleIndex& index, ArtifactId artifact);

} // namespace elysium::fortress
