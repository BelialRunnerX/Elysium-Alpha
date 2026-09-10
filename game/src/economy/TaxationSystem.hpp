// Intended function: Compute optional settlement tariffs, market fees, rent/service costs, treasury income, exemptions, and policy-driven distributions.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::economy {
struct TaxRecord {
    std::uint64_t recordId{};
    std::uint64_t payerId{};
    std::uint64_t treasuryId{};
    double amount{};
    std::uint64_t category{};
    std::uint64_t tick{};
};
class TaxRecordTable {
public:
 bool set(TaxRecord value); bool remove(std::uint64_t id);
 [[nodiscard]] const TaxRecord* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<TaxRecord> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const TaxRecord& value) noexcept; std::vector<TaxRecord> rows_;
};
}
