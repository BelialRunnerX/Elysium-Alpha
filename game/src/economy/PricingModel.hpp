// Intended function: Calculate deterministic local buy/sell price bands from base value, supply, demand, scarcity, risk, standing, and tariffs.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::economy {
struct PriceQuote {
    std::uint64_t quoteId{};
    std::uint64_t itemId{};
    double buyPrice{};
    double sellPrice{};
    double scarcity{};
    double risk{};
};
class PriceQuoteTable {
public:
 bool set(PriceQuote value); bool remove(std::uint64_t id);
 [[nodiscard]] const PriceQuote* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<PriceQuote> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const PriceQuote& value) noexcept; std::vector<PriceQuote> rows_;
};
}
