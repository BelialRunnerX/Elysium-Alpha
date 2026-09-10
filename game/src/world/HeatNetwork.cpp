// Intended function: Model bounded thermal-transfer links, heat sources, sinks, storage, and overheat protection for bases and industry.
#include "HeatNetwork.hpp"

namespace elysium::world {

std::uint64_t HeatLinkStore::keyOf(const HeatLink& value) noexcept { return static_cast<std::uint64_t>(value.stableId); }

bool HeatLinkStore::upsert(HeatLink value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const HeatLink& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool HeatLinkStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const HeatLink& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const HeatLink* HeatLinkStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const HeatLink& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<HeatLink> HeatLinkStore::ordered() const { return records_; }

} // namespace elysium::world
