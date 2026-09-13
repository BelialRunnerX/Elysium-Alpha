// Intended function: deterministic local reactions for lava/water and acid/material contact.
#pragma once

#include "world/BoundedFluidSolver.hpp"

#include <cstdint>

namespace elysium {

struct MaterialContact {
    LargeSurfaceCellAddress cell{};
    std::uint64_t materialKey{};
    double corrosionResistance01{};
    double neutralizationCapacity{};
};

struct MaterialReactionResult {
    double corrosionDamage{};
    double neutralizedAcid{};
    std::uint64_t replacementMaterialKey{};
};

class ReactiveFluidChemistry final {
public:
    [[nodiscard]] FluidReaction waterLava(
        const LargeSurfaceCellAddress& cell,
        double waterVolume,
        double lavaVolume,
        std::uint64_t cooledMaterialKey) const noexcept;

    [[nodiscard]] MaterialReactionResult acidContact(
        double acidVolume,
        const MaterialContact& material) const noexcept;
};

} // namespace elysium
