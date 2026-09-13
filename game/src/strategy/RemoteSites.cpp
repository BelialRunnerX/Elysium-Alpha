// Intended function: Represent off-screen settlements/outposts as compact stable strategic summaries for behavioral LOD simulation.
#include "RemoteSites.hpp"

namespace elysium::strategy {

std::uint64_t RemoteSiteStateStore::keyOf(const RemoteSiteState& value) noexcept { return static_cast<std::uint64_t>(value.siteId); }

bool RemoteSiteStateStore::upsert(RemoteSiteState value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const RemoteSiteState& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool RemoteSiteStateStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const RemoteSiteState& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const RemoteSiteState* RemoteSiteStateStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const RemoteSiteState& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<RemoteSiteState> RemoteSiteStateStore::ordered() const { return records_; }

} // namespace elysium::strategy
