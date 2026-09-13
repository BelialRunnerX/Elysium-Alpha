// Intended function: Represent blueprint placement, material staging, validation blockers, construction progress, and final world commits.
#include "BuildingSystem.hpp"

namespace elysium::gameplay {

std::uint64_t BuildJobStore::keyOf(const BuildJob& value) noexcept { return static_cast<std::uint64_t>(value.jobId); }

bool BuildJobStore::upsert(BuildJob value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const BuildJob& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool BuildJobStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const BuildJob& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const BuildJob* BuildJobStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const BuildJob& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<BuildJob> BuildJobStore::ordered() const { return records_; }

} // namespace elysium::gameplay
