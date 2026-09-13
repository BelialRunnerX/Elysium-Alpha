// Intended function: Implement player-facing mining progress, tool efficiency, deterministic ore rewards, durability, and committed world-edit requests.
#include "MiningSystem.hpp"

namespace elysium::gameplay {

std::uint64_t MiningActionStore::keyOf(const MiningAction& value) noexcept { return static_cast<std::uint64_t>(value.actionId); }

bool MiningActionStore::upsert(MiningAction value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const MiningAction& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool MiningActionStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const MiningAction& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const MiningAction* MiningActionStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const MiningAction& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<MiningAction> MiningActionStore::ordered() const { return records_; }

} // namespace elysium::gameplay
