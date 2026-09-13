// Intended function: Track equipped stable items, slot compatibility, condition, armor/suit layers, and derived loadout readiness.
#include "EquipmentSystem.hpp"

namespace elysium::gameplay {

std::uint64_t EquipmentSlotStateStore::keyOf(const EquipmentSlotState& value) noexcept { return static_cast<std::uint64_t>(value.slotId); }

bool EquipmentSlotStateStore::upsert(EquipmentSlotState value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const EquipmentSlotState& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool EquipmentSlotStateStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const EquipmentSlotState& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const EquipmentSlotState* EquipmentSlotStateStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const EquipmentSlotState& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<EquipmentSlotState> EquipmentSlotStateStore::ordered() const { return records_; }

} // namespace elysium::gameplay
