// Intended function: Queue 16^3 microvoxel edits, damage masks, refinement ownership, and deterministic commit ordering.
#include "MicroVoxelEditing.hpp"

namespace elysium::world {

std::uint64_t MicroEditStore::keyOf(const MicroEdit& value) noexcept { return static_cast<std::uint64_t>(value.editId); }

bool MicroEditStore::upsert(MicroEdit value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const MicroEdit& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool MicroEditStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const MicroEdit& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const MicroEdit* MicroEditStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const MicroEdit& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<MicroEdit> MicroEditStore::ordered() const { return records_; }

} // namespace elysium::world
