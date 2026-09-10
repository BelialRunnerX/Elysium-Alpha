// Intended function: Track Favor, Suspicion, warrants, registration state, decay rules, and threshold-triggered Imperial consequences.
#include "EmpireStanding.hpp"

namespace elysium::strategy {

std::uint64_t StandingRecordStore::keyOf(const StandingRecord& value) noexcept { return static_cast<std::uint64_t>(value.systemId); }

bool StandingRecordStore::upsert(StandingRecord value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const StandingRecord& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool StandingRecordStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const StandingRecord& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const StandingRecord* StandingRecordStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const StandingRecord& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<StandingRecord> StandingRecordStore::ordered() const { return records_; }

} // namespace elysium::strategy
