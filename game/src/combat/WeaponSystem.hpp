// Intended function: Describe weapon runtime state, firing cadence, heat/ammo, alt-fire modes, durability, and deterministic shot requests.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::combat {

struct WeaponRuntime {
    std::uint64_t weaponId{};
    std::uint64_t ammo{};
    double heat{};
    double cooldown{};
    double condition{};
    std::uint64_t mode{};
};

class WeaponRuntimeStore {
public:
    bool upsert(WeaponRuntime value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const WeaponRuntime* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<WeaponRuntime> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const WeaponRuntime& value) noexcept;
    std::vector<WeaponRuntime> records_;
};

} // namespace elysium::combat
