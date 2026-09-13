// Intended function: Manage death/knockout recovery, beacon-based respawn choices, inventory penalties, and persistent consequence records.
#include "RespawnSystem.hpp"

namespace elysium::gameplay {

std::uint64_t RespawnOptionStore::keyOf(const RespawnOption& value) noexcept { return static_cast<std::uint64_t>(value.siteId); }

bool RespawnOptionStore::upsert(RespawnOption value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const RespawnOption& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool RespawnOptionStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const RespawnOption& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const RespawnOption* RespawnOptionStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const RespawnOption& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<RespawnOption> RespawnOptionStore::ordered() const { return records_; }

} // namespace elysium::gameplay
