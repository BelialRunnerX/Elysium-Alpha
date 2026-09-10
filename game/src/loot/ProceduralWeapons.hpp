// Intended function: Generate stable procedural weapon variants from archetype, material, barrel/core, element, affixes, and rarity budget.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::loot {
struct ProceduralWeapon {
    std::uint64_t weaponId{};
    std::uint64_t archetypeId{};
    std::uint64_t materialId{};
    std::uint64_t elementId{};
    double power{};
    std::uint64_t seed{};
};
class ProceduralWeaponStore {
public:
 bool put(ProceduralWeapon v); bool erase(std::uint64_t id);
 [[nodiscard]] const ProceduralWeapon* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<ProceduralWeapon>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const ProceduralWeapon& v) noexcept; std::vector<ProceduralWeapon> values_;
};
} // namespace elysium::loot
