// Intended function: Model deterministic planetary strata, local geology samples, and ore-bearing layer metadata for mining/worldgen.
#include "GeologyLayers.hpp"

namespace elysium::world {

std::uint64_t GeologyLayerStore::keyOf(const GeologyLayer& value) noexcept { return static_cast<std::uint64_t>(value.layerId); }

bool GeologyLayerStore::upsert(GeologyLayer value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const GeologyLayer& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool GeologyLayerStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const GeologyLayer& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const GeologyLayer* GeologyLayerStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const GeologyLayer& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<GeologyLayer> GeologyLayerStore::ordered() const { return records_; }

} // namespace elysium::world
