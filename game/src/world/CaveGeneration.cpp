// Intended function: Describe deterministic cave-carving nodes and tunnel links for chunk-local generation and later meshing.
#include "CaveGeneration.hpp"

namespace elysium::world {

std::uint64_t CaveNodeStore::keyOf(const CaveNode& value) noexcept { return static_cast<std::uint64_t>(value.stableId); }

bool CaveNodeStore::upsert(CaveNode value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const CaveNode& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool CaveNodeStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const CaveNode& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const CaveNode* CaveNodeStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const CaveNode& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<CaveNode> CaveNodeStore::ordered() const { return records_; }

} // namespace elysium::world
