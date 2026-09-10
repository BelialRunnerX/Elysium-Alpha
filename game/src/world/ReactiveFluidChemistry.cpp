// Intended function: bounded, clamped fluid reaction math; mutation remains with caller.
#include "world/ReactiveFluidChemistry.hpp"

#include "core/Saturating.hpp"

#include <algorithm>

namespace elysium {

FluidReaction ReactiveFluidChemistry::waterLava(
    const LargeSurfaceCellAddress& cell,
    double waterVolume,
    double lavaVolume,
    std::uint64_t cooledMaterialKey) const noexcept {
    waterVolume = safe::finiteClamp(waterVolume, 0.0, 1.0);
    lavaVolume = safe::finiteClamp(lavaVolume, 0.0, 1.0);
    const double reacted = std::min(waterVolume, lavaVolume);
    return {
        .cell = cell,
        .a = FluidKind::Water,
        .b = FluidKind::Lava,
        .consumedA = reacted,
        .consumedB = reacted,
        .productMaterialKey = cooledMaterialKey,
        .heatDelta = safe::nonNegative(reacted * 900.0),
    };
}

MaterialReactionResult ReactiveFluidChemistry::acidContact(
    double acidVolume,
    const MaterialContact& material) const noexcept {
    acidVolume = safe::finiteClamp(acidVolume, 0.0, 1.0);
    const double resistance = safe::finiteClamp(material.corrosionResistance01, 0.0, 1.0);
    const double neutralization = safe::nonNegative(material.neutralizationCapacity);
    const double neutralized = safe::finiteClamp(std::min(acidVolume, neutralization), 0.0, 1.0);
    const double active = safe::finiteClamp(acidVolume - neutralized, 0.0, 1.0);
    const double damage = safe::nonNegative(active * (1.0 - resistance));
    return {
        .corrosionDamage = damage,
        .neutralizedAcid = neutralized,
        .replacementMaterialKey = damage >= 1.0 ? 0ULL : material.materialKey,
    };
}

} // namespace elysium
