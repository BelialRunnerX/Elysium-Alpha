// Intended function: Drive data-oriented crafting jobs with recipe identity, reserved inputs, progress, output quality, and cancellation.
#include "CraftingSystem.hpp"

namespace elysium::gameplay {

std::uint64_t CraftingJobStore::keyOf(const CraftingJob& value) noexcept { return static_cast<std::uint64_t>(value.jobId); }

bool CraftingJobStore::upsert(CraftingJob value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const CraftingJob& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool CraftingJobStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const CraftingJob& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const CraftingJob* CraftingJobStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const CraftingJob& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<CraftingJob> CraftingJobStore::ordered() const { return records_; }

} // namespace elysium::gameplay
