// Intended function: Describe weapon runtime state, firing cadence, heat/ammo, alt-fire modes, durability, and deterministic shot requests.
#include "WeaponSystem.hpp"

namespace elysium::combat {

std::uint64_t WeaponRuntimeStore::keyOf(const WeaponRuntime& value) noexcept { return static_cast<std::uint64_t>(value.weaponId); }

bool WeaponRuntimeStore::upsert(WeaponRuntime value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const WeaponRuntime& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool WeaponRuntimeStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const WeaponRuntime& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const WeaponRuntime* WeaponRuntimeStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const WeaponRuntime& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<WeaponRuntime> WeaponRuntimeStore::ordered() const { return records_; }

} // namespace elysium::combat
