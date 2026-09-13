#include "fortress/StockIndex.hpp"

#include <algorithm>

namespace elysium::fortress {

void StockIndex::rebuild(std::span<const StockIndexEntry> entries) {
    entries_.clear();
    byContent_.clear();
    for (const auto& entry : entries) upsert(entry);
}

void StockIndex::upsert(StockIndexEntry entry) {
    const auto old = entries_.find(entry.item.value);
    if (old != entries_.end() && old->second.content != entry.content) {
        auto& ids = byContent_[old->second.content.value];
        std::erase(ids, entry.item);
    }
    entries_[entry.item.value] = entry;
    auto& ids = byContent_[entry.content.value];
    if (std::find(ids.begin(), ids.end(), entry.item) == ids.end()) {
        ids.push_back(entry.item);
        std::sort(ids.begin(), ids.end(), [](StableId a, StableId b) { return a.value < b.value; });
    }
}

void StockIndex::erase(StableId item) {
    const auto it = entries_.find(item.value);
    if (it == entries_.end()) return;
    const auto content = it->second.content.value;
    entries_.erase(it);
    auto list = byContent_.find(content);
    if (list == byContent_.end()) return;
    std::erase(list->second, item);
    if (list->second.empty()) byContent_.erase(list);
}

float StockIndex::available(std::string_view content, SiteId site) const {
    const auto it = byContent_.find(std::string(content));
    if (it == byContent_.end()) return 0.0f;
    float total = 0.0f;
    for (const auto id : it->second) {
        const auto entry = entries_.find(id.value);
        if (entry == entries_.end() || entry->second.site != site || entry->second.forbidden) continue;
        total += std::max(0.0f, entry->second.quantity - entry->second.reservedQuantity);
    }
    return total;
}

std::vector<StableId> StockIndex::findAvailable(std::string_view content,
                                                SiteId site,
                                                float requiredQuantity,
                                                std::size_t limit) const {
    std::vector<StableId> result;
    const auto it = byContent_.find(std::string(content));
    if (it == byContent_.end()) return result;
    float gathered = 0.0f;
    for (const auto id : it->second) {
        const auto entry = entries_.find(id.value);
        if (entry == entries_.end() || entry->second.site != site || entry->second.forbidden) continue;
        const float availableQuantity = std::max(0.0f, entry->second.quantity - entry->second.reservedQuantity);
        if (availableQuantity <= 0.0f) continue;
        result.push_back(id);
        gathered += availableQuantity;
        if (result.size() >= limit || gathered >= requiredQuantity) break;
    }
    return result;
}

} // namespace elysium::fortress
