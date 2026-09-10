// Intended function: Maintain deterministic strategic trade-route summaries, throughput, risk, tariffs, supply pressure, and disruption.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::strategy {

struct TradeRoute {
    std::uint64_t routeId{};
    std::uint64_t originId{};
    std::uint64_t destinationId{};
    double capacity{};
    double risk{};
    double tariff{};
};

class TradeRouteStore {
public:
    bool upsert(TradeRoute value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const TradeRoute* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<TradeRoute> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const TradeRoute& value) noexcept;
    std::vector<TradeRoute> records_;
};

} // namespace elysium::strategy
