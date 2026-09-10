// Intended function: Select stable combat targets using threat, visibility, range, faction, cover, and deterministic tie-breaks.
#include "TargetingSystem.hpp"

namespace elysium::combat {

std::uint64_t TargetCandidateStore::keyOf(const TargetCandidate& value) noexcept { return static_cast<std::uint64_t>(value.stableId); }

bool TargetCandidateStore::upsert(TargetCandidate value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const TargetCandidate& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool TargetCandidateStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const TargetCandidate& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const TargetCandidate* TargetCandidateStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const TargetCandidate& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<TargetCandidate> TargetCandidateStore::ordered() const { return records_; }

} // namespace elysium::combat
