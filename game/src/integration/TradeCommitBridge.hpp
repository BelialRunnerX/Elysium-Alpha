// Intended function: Translate evaluated strategic trade into atomic local item/credit/ownership commit batches before settlement acknowledgement.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::integration {
struct TradeCommitIntent {
    std::uint64_t intentId{};
    std::uint64_t manifestId{};
    std::uint64_t buyerId{};
    std::uint64_t sellerId{};
    std::uint64_t assetHash{};
    double value{};
};
class TradeCommitIntentIndex {
public:
 bool upsert(TradeCommitIntent value); bool erase(std::uint64_t id); [[nodiscard]] const TradeCommitIntent* find(std::uint64_t id) const; [[nodiscard]] const std::vector<TradeCommitIntent>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const TradeCommitIntent& value) noexcept; std::vector<TradeCommitIntent> rows_;
};
}
