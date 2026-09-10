// Intended function: Project vehicles/ships with hull/module state, damage, lights, thrusters, cargo attachments, doors, and occupant markers.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::render {
struct VehicleRenderState {
    std::uint64_t stableId{};
    std::uint64_t hullId{};
    std::uint64_t moduleHash{};
    std::uint64_t damageHash{};
    std::uint64_t effectMask{};
    std::uint64_t flags{};
};
class VehicleRenderStateCollection {
public:
 bool store(VehicleRenderState value); bool erase(std::uint64_t id); [[nodiscard]] const VehicleRenderState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<VehicleRenderState>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const VehicleRenderState& v) noexcept; std::vector<VehicleRenderState> rows_;
};
}
