// Intended function: Track persistent operative experience, levels, unlock points, milestone flags, and deterministic progression rewards.
#include "PlayerProgression.hpp"

namespace elysium::gameplay {

std::uint64_t ProgressionTrackStore::keyOf(const ProgressionTrack& value) noexcept { return static_cast<std::uint64_t>(value.trackId); }

bool ProgressionTrackStore::upsert(ProgressionTrack value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const ProgressionTrack& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool ProgressionTrackStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const ProgressionTrack& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const ProgressionTrack* ProgressionTrackStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const ProgressionTrack& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<ProgressionTrack> ProgressionTrackStore::ordered() const { return records_; }

} // namespace elysium::gameplay
