// Intended function: Plan atomic source/destination/ownership container transfers with capacity, filter, reservation, hazard, and rollback checks.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::inventory {
struct TransferPlan {
    std::uint64_t planId{};
    std::uint64_t sourceId{};
    std::uint64_t destinationId{};
    std::uint64_t assetId{};
    std::uint64_t units{};
    std::uint64_t state{};
};
class TransferPlanIndex {
public:
 bool upsert(TransferPlan value); bool erase(std::uint64_t id); [[nodiscard]] const TransferPlan* find(std::uint64_t id) const; [[nodiscard]] const std::vector<TransferPlan>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const TransferPlan& value) noexcept; std::vector<TransferPlan> rows_;
};
}
