// Intended function: Track research nodes, prerequisites, costs, unlock effects, repeatable levels, and deterministic discovery gating.
#include "TechTree.hpp"

namespace elysium::research {

std::uint64_t TechNodeStateStore::keyOf(const TechNodeState& value) noexcept { return static_cast<std::uint64_t>(value.techId); }

bool TechNodeStateStore::upsert(TechNodeState value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const TechNodeState& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool TechNodeStateStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const TechNodeState& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const TechNodeState* TechNodeStateStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const TechNodeState& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<TechNodeState> TechNodeStateStore::ordered() const { return records_; }

} // namespace elysium::research
