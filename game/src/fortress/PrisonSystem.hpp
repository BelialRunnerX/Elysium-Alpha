// Intended function: Manage detention cells, custody assignments, sentence clocks, transfer/exile requests, and prisoner welfare.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::fortress {
struct CustodyState {
    std::uint64_t prisonerId{};
    std::uint64_t caseId{};
    std::uint64_t cellId{};
    std::uint64_t sentenceTicks{};
    std::uint64_t servedTicks{};
    std::uint64_t status{};
};
class CustodyStateStore {
public:
 bool put(CustodyState v); bool erase(std::uint64_t id);
 [[nodiscard]] const CustodyState* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<CustodyState>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const CustodyState& v) noexcept; std::vector<CustodyState> values_;
};
} // namespace elysium::fortress
