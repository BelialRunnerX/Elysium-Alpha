// Intended function: Track fire-response zones, suppressant stores, vent isolation, and bounded emergency suppression intents.
#include "FireSuppression.hpp"

namespace elysium::world {

std::uint64_t SuppressionZoneStore::keyOf(const SuppressionZone& value) noexcept { return static_cast<std::uint64_t>(value.zoneId); }

bool SuppressionZoneStore::upsert(SuppressionZone value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const SuppressionZone& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool SuppressionZoneStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const SuppressionZone& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const SuppressionZone* SuppressionZoneStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const SuppressionZone& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<SuppressionZone> SuppressionZoneStore::ordered() const { return records_; }

} // namespace elysium::world
