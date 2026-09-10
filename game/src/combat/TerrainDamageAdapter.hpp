#pragma once
#include "combat/CombatRules.hpp"
#include <cstdint>
namespace elysium::combat {
struct TerrainHitContext{std::uint64_t eventId{},attackerId{},targetAddress{};TerrainDamageCategory category{TerrainDamageCategory::None};double landedDamage{},toolBreachScale{1};};
struct TerrainDamageRequest{std::uint64_t eventId{},attackerId{},targetAddress{};double structuralDamage{};bool allowMicrodetail{},allowExcavation{},allowStructuralBreach{};};
class TerrainDamageAdapter{public:[[nodiscard]]TerrainDamageRequest translate(const TerrainHitContext&c)const;};
} // namespace elysium::combat
