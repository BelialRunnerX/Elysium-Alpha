// Intended function: Track personal drone follow/hold/scout/mine/repair/haul modes, energy, inventory, damage, and recall behavior.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::gameplay {
struct CompanionDroneState {
    std::uint64_t droneId{};
    std::uint64_t ownerId{};
    std::uint64_t mode{};
    double energy{};
    double condition{};
    double targetId{};
};
class CompanionDroneStateStore {
public:
 bool put(CompanionDroneState v); bool erase(std::uint64_t id);
 [[nodiscard]] const CompanionDroneState* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<CompanionDroneState>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const CompanionDroneState& v) noexcept; std::vector<CompanionDroneState> values_;
};
} // namespace elysium::gameplay
