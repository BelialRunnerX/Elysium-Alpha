// Intended function: Model stable fluid/atmosphere pipe segments and deterministic local flow requests between bounded endpoints.
#include "PipeNetwork.hpp"

namespace elysium::world {

std::uint64_t PipeSegmentStore::keyOf(const PipeSegment& value) noexcept { return static_cast<std::uint64_t>(value.stableId); }

bool PipeSegmentStore::upsert(PipeSegment value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const PipeSegment& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool PipeSegmentStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const PipeSegment& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const PipeSegment* PipeSegmentStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const PipeSegment& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<PipeSegment> PipeSegmentStore::ordered() const { return records_; }

} // namespace elysium::world
