// Intended function: Track stable account balances, reservations, transfers, fees, rewards, debts, and exactly-once settlement references.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::economy {
struct AccountState {
    std::uint64_t accountId{};
    double balance{};
    double reserved{};
    double debt{};
    std::uint64_t revision{};
    std::uint64_t flags{};
};
class AccountStateTable {
public:
 bool set(AccountState value); bool remove(std::uint64_t id);
 [[nodiscard]] const AccountState* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<AccountState> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const AccountState& value) noexcept; std::vector<AccountState> rows_;
};
}
