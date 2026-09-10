// Intended function: Queue deterministic stable-ID commands sorted by kind/key/sequence rather than worker arrival order.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::simulation {
struct StableCommand {
    std::uint64_t commandId{};
    std::uint64_t kind{};
    std::uint64_t primaryKey{};
    std::uint64_t secondaryKey{};
    std::uint64_t sequence{};
    std::uint64_t flags{};
};
class StableCommandTable {
public:
 bool set(StableCommand value); bool remove(std::uint64_t id);
 [[nodiscard]] const StableCommand* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<StableCommand> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const StableCommand& value) noexcept; std::vector<StableCommand> rows_;
};
}
