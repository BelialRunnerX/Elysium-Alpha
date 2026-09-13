// Intended function: Represent atomic ownership transfer intents for items, cargo, structures, vehicles, artifacts, and trade settlements.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::economy {
struct OwnershipTransferState {
    std::uint64_t transferId{};
    std::uint64_t assetId{};
    std::uint64_t fromOwner{};
    std::uint64_t toOwner{};
    std::uint64_t state{};
    std::uint64_t tick{};
};
class OwnershipTransferStateTable {
public:
 bool set(OwnershipTransferState value); bool remove(std::uint64_t id);
 [[nodiscard]] const OwnershipTransferState* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<OwnershipTransferState> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const OwnershipTransferState& value) noexcept; std::vector<OwnershipTransferState> rows_;
};
}
