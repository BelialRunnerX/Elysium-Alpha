// Intended function: Translate machine/local-network inventory batches into physical-item ledger promotion/reservation/delivery requests when identity is required.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::integration {
struct IndustryItemIntent {
    std::uint64_t intentId{};
    std::uint64_t machineId{};
    std::uint64_t itemId{};
    std::uint64_t units{};
    std::uint64_t reason{};
    std::uint64_t state{};
};
class IndustryItemIntentIndex {
public:
 bool upsert(IndustryItemIntent value); bool erase(std::uint64_t id); [[nodiscard]] const IndustryItemIntent* find(std::uint64_t id) const; [[nodiscard]] const std::vector<IndustryItemIntent>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const IndustryItemIntent& value) noexcept; std::vector<IndustryItemIntent> rows_;
};
}
