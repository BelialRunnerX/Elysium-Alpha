// Intended function: Project market quotes, manifests, legality, ownership, tariffs, standing effects, settlement state, and transfer blockers.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ui {
struct TradeViewState {
    std::uint64_t viewId{};
    std::uint64_t marketId{};
    std::uint64_t manifestId{};
    double totalValue{};
    double risk{};
    std::uint64_t flags{};
};
class TradeViewStateTable {
public:
 bool set(TradeViewState value); bool remove(std::uint64_t id);
 [[nodiscard]] const TradeViewState* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<TradeViewState> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const TradeViewState& value) noexcept; std::vector<TradeViewState> rows_;
};
}
