// Intended function: Resolve layered armor/suit mitigation, condition loss, penetration, breach flags, and repair requirements.
#include "ArmorSystem.hpp"

namespace elysium::combat {

std::uint64_t ArmorLayerStore::keyOf(const ArmorLayer& value) noexcept { return static_cast<std::uint64_t>(value.itemId); }

bool ArmorLayerStore::upsert(ArmorLayer value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const ArmorLayer& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool ArmorLayerStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const ArmorLayer& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const ArmorLayer* ArmorLayerStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const ArmorLayer& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<ArmorLayer> ArmorLayerStore::ordered() const { return records_; }

} // namespace elysium::combat
