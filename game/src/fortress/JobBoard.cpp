#include "fortress/JobBoard.hpp"

#include <algorithm>

namespace elysium::fortress {

void JobBoard::upsert(JobBoardEntry entry) {
    if (!entry.id) return;
    const auto existing = entries_.find(entry.id.value);
    if (existing != entries_.end() && existing->second.labor != entry.labor) {
        auto& ids = byLabor_[existing->second.labor.value];
        std::erase(ids, entry.id);
    }
    entries_[entry.id.value] = entry;
    auto& ids = byLabor_[entry.labor.value];
    if (std::find(ids.begin(), ids.end(), entry.id) == ids.end()) {
        ids.push_back(entry.id);
        std::sort(ids.begin(), ids.end(), [](JobId a, JobId b) { return a.value < b.value; });
    }
}

void JobBoard::erase(JobId id) {
    const auto it = entries_.find(id.value);
    if (it == entries_.end()) return;
    const auto labor = it->second.labor.value;
    entries_.erase(it);
    auto mapIt = byLabor_.find(labor);
    if (mapIt == byLabor_.end()) return;
    std::erase(mapIt->second, id);
    if (mapIt->second.empty()) byLabor_.erase(mapIt);
}

const JobBoardEntry* JobBoard::find(JobId id) const {
    const auto it = entries_.find(id.value);
    return it == entries_.end() ? nullptr : &it->second;
}

std::vector<JobId> JobBoard::candidates(std::string_view labor,
                                       PriorityBand minimumPriority,
                                       SiteId site,
                                       std::size_t limit) const {
    std::vector<JobBoardEntry> filtered;
    const auto it = byLabor_.find(std::string(labor));
    if (it == byLabor_.end()) return {};
    for (const auto id : it->second) {
        const auto* entry = find(id);
        if (entry == nullptr || entry->site != site || entry->worker ||
            entry->priority < minimumPriority ||
            (entry->state != JobState::Pending && entry->state != JobState::Blocked)) {
            continue;
        }
        filtered.push_back(*entry);
    }
    std::sort(filtered.begin(), filtered.end(), [](const JobBoardEntry& a, const JobBoardEntry& b) {
        if (a.priority != b.priority) return a.priority > b.priority;
        return a.id.value < b.id.value;
    });
    std::vector<JobId> result;
    result.reserve(std::min(limit, filtered.size()));
    for (std::size_t i = 0; i < filtered.size() && i < limit; ++i) result.push_back(filtered[i].id);
    return result;
}

} // namespace elysium::fortress
