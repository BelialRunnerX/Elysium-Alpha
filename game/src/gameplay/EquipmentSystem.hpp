// Intended function: Track equipped stable items, slot compatibility, condition, armor/suit layers, and derived loadout readiness.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::gameplay {

struct EquipmentSlotState {
    std::uint64_t slotId{};
    std::uint64_t itemStableId{};
    std::uint64_t contentId{};
    double condition{};
    std::uint64_t layer{};
    std::uint64_t flags{};
};

class EquipmentSlotStateStore {
public:
    bool upsert(EquipmentSlotState value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const EquipmentSlotState* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<EquipmentSlotState> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const EquipmentSlotState& value) noexcept;
    std::vector<EquipmentSlotState> records_;
};

} // namespace elysium::gameplay
