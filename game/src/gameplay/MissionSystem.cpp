// Intended function: Track multi-stage objectives, optional branches, timers, rewards, and Chronicle-significant mission outcomes.
#include "MissionSystem.hpp"

namespace elysium::gameplay {

std::uint64_t MissionStateStore::keyOf(const MissionState& value) noexcept { return static_cast<std::uint64_t>(value.missionId); }

bool MissionStateStore::upsert(MissionState value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const MissionState& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool MissionStateStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const MissionState& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const MissionState* MissionStateStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const MissionState& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<MissionState> MissionStateStore::ordered() const { return records_; }

} // namespace elysium::gameplay
