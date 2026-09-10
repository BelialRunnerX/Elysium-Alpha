// Intended function: Track installed vehicle modules, slots, condition, power draw, mass, and enabled state.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::travel {
struct VehicleModuleState {
    std::uint64_t vehicleId{};
    std::uint64_t moduleId{};
    std::uint64_t slotId{};
    double condition{};
    double powerDraw{};
    std::uint64_t enabled{};
};
class VehicleModuleStateRegistry {
public:
    bool publish(VehicleModuleState record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const VehicleModuleState* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<VehicleModuleState>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const VehicleModuleState& r) noexcept;
    std::vector<VehicleModuleState> records_;
};
} // namespace elysium::travel
