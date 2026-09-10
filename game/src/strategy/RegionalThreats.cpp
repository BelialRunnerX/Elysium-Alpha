// Intended function: Aggregate strategic threats by region, escalation, mobility, known intelligence, and response pressure.
#include "RegionalThreats.hpp"

namespace elysium::strategy {

std::uint64_t RegionalThreatStore::keyOf(const RegionalThreat& value) noexcept { return static_cast<std::uint64_t>(value.threatId); }

bool RegionalThreatStore::upsert(RegionalThreat value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const RegionalThreat& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool RegionalThreatStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const RegionalThreat& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const RegionalThreat* RegionalThreatStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const RegionalThreat& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<RegionalThreat> RegionalThreatStore::ordered() const { return records_; }

} // namespace elysium::strategy
