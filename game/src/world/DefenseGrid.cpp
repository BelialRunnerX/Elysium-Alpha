// Intended function: Coordinate defensive emplacements, sectors, ammunition readiness, power readiness, and tactical firing policy.
#include "DefenseGrid.hpp"

namespace elysium::world {

std::uint64_t DefenseNodeStore::keyOf(const DefenseNode& value) noexcept { return static_cast<std::uint64_t>(value.stableId); }

bool DefenseNodeStore::upsert(DefenseNode value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const DefenseNode& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool DefenseNodeStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const DefenseNode& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const DefenseNode* DefenseNodeStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const DefenseNode& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<DefenseNode> DefenseNodeStore::ordered() const { return records_; }

} // namespace elysium::world
