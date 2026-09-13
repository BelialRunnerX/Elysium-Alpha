// Intended function: Maintain deterministic strategic trade-route summaries, throughput, risk, tariffs, supply pressure, and disruption.
#include "TradeRoutes.hpp"

namespace elysium::strategy {

std::uint64_t TradeRouteStore::keyOf(const TradeRoute& value) noexcept { return static_cast<std::uint64_t>(value.routeId); }

bool TradeRouteStore::upsert(TradeRoute value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const TradeRoute& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool TradeRouteStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const TradeRoute& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const TradeRoute* TradeRouteStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const TradeRoute& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<TradeRoute> TradeRouteStore::ordered() const { return records_; }

} // namespace elysium::strategy
