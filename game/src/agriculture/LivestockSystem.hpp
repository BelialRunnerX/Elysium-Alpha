// Intended function: Manage domestic animals, feed, water, shelter, breeding, products, health, slaughter, and population pressure.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::agriculture {
struct LivestockState {
    std::uint64_t animalId{};
    std::uint64_t speciesId{};
    double health{};
    double hunger{};
    double reproduction{};
    std::uint64_t status{};
};
class LivestockStateStore {
public:
 bool put(LivestockState v); bool erase(std::uint64_t id);
 [[nodiscard]] const LivestockState* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<LivestockState>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const LivestockState& v) noexcept; std::vector<LivestockState> values_;
};
} // namespace elysium::agriculture
