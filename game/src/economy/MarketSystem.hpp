// Intended function: Maintain bounded local market offers, demand signals, stock pressure, service fees, and deterministic refresh epochs.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::economy {
struct MarketOffer {
    std::uint64_t offerId{};
    std::uint64_t marketId{};
    std::uint64_t itemId{};
    std::uint64_t units{};
    double unitPrice{};
    std::uint64_t expiresTick{};
};
class MarketOfferTable {
public:
 bool set(MarketOffer value); bool remove(std::uint64_t id);
 [[nodiscard]] const MarketOffer* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<MarketOffer> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const MarketOffer& value) noexcept; std::vector<MarketOffer> rows_;
};
}
