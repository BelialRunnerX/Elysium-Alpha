// Intended function: Represent waste streams, contamination classes, recycling queues, and safe disposal/reclamation outputs.
#include "WasteProcessing.hpp"

namespace elysium::world {

std::uint64_t WasteBatchStore::keyOf(const WasteBatch& value) noexcept { return static_cast<std::uint64_t>(value.stableId); }

bool WasteBatchStore::upsert(WasteBatch value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const WasteBatch& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool WasteBatchStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const WasteBatch& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const WasteBatch* WasteBatchStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const WasteBatch& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<WasteBatch> WasteBatchStore::ordered() const { return records_; }

} // namespace elysium::world
