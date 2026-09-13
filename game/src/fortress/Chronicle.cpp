#include "fortress/Chronicle.hpp"

#include <algorithm>

namespace elysium::fortress {
namespace {
void addRefs(ChronicleIndex& index, std::size_t i) {
    const auto& event = index.events[i];
    if (event.primary) index.byFigure[event.primary.value].push_back(i);
    if (event.secondary) index.byFigure[event.secondary.value].push_back(i);
    if (event.site) index.bySite[event.site.value].push_back(i);
    if (event.artifact) index.byArtifact[event.artifact.value].push_back(i);
    if (event.organization) index.byOrganization[event.organization.value].push_back(i);
}

template <class Map, class Key>
std::vector<const HistoricalEvent*> fetch(const ChronicleIndex& index, const Map& map, Key key) {
    std::vector<const HistoricalEvent*> result;
    const auto it = map.find(key.value);
    if (it == map.end()) return result;
    result.reserve(it->second.size());
    for (const auto i : it->second) if (i < index.events.size()) result.push_back(&index.events[i]);
    std::stable_sort(result.begin(), result.end(), [](const HistoricalEvent* a, const HistoricalEvent* b) {
        if (a->time.year != b->time.year) return a->time.year < b->time.year;
        if (a->time.day != b->time.day) return a->time.day < b->time.day;
        return a->time.tick < b->time.tick;
    });
    return result;
}
}

void rebuildChronicleIndex(ChronicleIndex& index) {
    index.byFigure.clear();
    index.bySite.clear();
    index.byArtifact.clear();
    index.byOrganization.clear();
    for (std::size_t i = 0; i < index.events.size(); ++i) addRefs(index, i);
}

void appendChronicleEvent(ChronicleIndex& index, HistoricalEvent event) {
    const std::size_t i = index.events.size();
    index.events.push_back(std::move(event));
    addRefs(index, i);
}

std::vector<const HistoricalEvent*> eventsForFigure(const ChronicleIndex& index, StableId figure) {
    return fetch(index, index.byFigure, figure);
}

std::vector<const HistoricalEvent*> eventsForSite(const ChronicleIndex& index, SiteId site) {
    return fetch(index, index.bySite, site);
}

std::vector<const HistoricalEvent*> eventsForArtifact(const ChronicleIndex& index, ArtifactId artifact) {
    return fetch(index, index.byArtifact, artifact);
}

} // namespace elysium::fortress
