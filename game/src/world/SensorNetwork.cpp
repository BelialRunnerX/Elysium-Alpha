// Intended function: Aggregate local sensor nodes into bounded contact reports used by defense, automation, alerts, and exploration.
#include "SensorNetwork.hpp"

namespace elysium::world {

std::uint64_t SensorContactStore::keyOf(const SensorContact& value) noexcept { return static_cast<std::uint64_t>(value.contactId); }

bool SensorContactStore::upsert(SensorContact value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const SensorContact& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool SensorContactStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const SensorContact& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const SensorContact* SensorContactStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const SensorContact& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<SensorContact> SensorContactStore::ordered() const { return records_; }

} // namespace elysium::world
