// Intended function: Track stable room identities, ownership, functional tags, capacity, and reassignment requests for fortress spaces.
#include "RoomAssignment.hpp"

namespace elysium::world {

std::uint64_t RoomAssignmentStore::keyOf(const RoomAssignment& value) noexcept { return static_cast<std::uint64_t>(value.roomId); }

bool RoomAssignmentStore::upsert(RoomAssignment value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const RoomAssignment& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool RoomAssignmentStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const RoomAssignment& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const RoomAssignment* RoomAssignmentStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const RoomAssignment& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<RoomAssignment> RoomAssignmentStore::ordered() const { return records_; }

} // namespace elysium::world
