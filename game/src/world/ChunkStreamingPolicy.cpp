// Intended function: Prioritize sparse chunk residency using player distance, active jobs, machines, hazards, edits, and save pressure.
#include "ChunkStreamingPolicy.hpp"

namespace elysium::world {

std::uint64_t ChunkResidencyRequestStore::keyOf(const ChunkResidencyRequest& value) noexcept { return static_cast<std::uint64_t>(value.addressKey); }

bool ChunkResidencyRequestStore::upsert(ChunkResidencyRequest value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const ChunkResidencyRequest& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool ChunkResidencyRequestStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const ChunkResidencyRequest& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const ChunkResidencyRequest* ChunkResidencyRequestStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const ChunkResidencyRequest& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<ChunkResidencyRequest> ChunkResidencyRequestStore::ordered() const { return records_; }

} // namespace elysium::world
