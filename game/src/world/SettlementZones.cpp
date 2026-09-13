// Intended function: Represent bounded policy zones such as stockpiles, hospitals, barracks, farms, traffic, and emergency districts.
#include "SettlementZones.hpp"

namespace elysium::world {

std::uint64_t SettlementZoneStore::keyOf(const SettlementZone& value) noexcept { return static_cast<std::uint64_t>(value.zoneId); }

bool SettlementZoneStore::upsert(SettlementZone value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const SettlementZone& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool SettlementZoneStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const SettlementZone& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const SettlementZone* SettlementZoneStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const SettlementZone& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<SettlementZone> SettlementZoneStore::ordered() const { return records_; }

} // namespace elysium::world
