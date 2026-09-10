// Intended function: Coordinate stable entity promotion/demotion between strategic summaries and active ECS shards without identity loss.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::simulation {
struct ShardTransfer {
    std::uint64_t transferId{};
    std::uint64_t stableId{};
    std::uint64_t fromShard{};
    std::uint64_t toShard{};
    std::uint64_t stateHash{};
    std::uint64_t tick{};
};
class ShardTransferTable {
public:
 bool set(ShardTransfer value); bool remove(std::uint64_t id);
 [[nodiscard]] const ShardTransfer* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<ShardTransfer> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const ShardTransfer& value) noexcept; std::vector<ShardTransfer> rows_;
};
}
