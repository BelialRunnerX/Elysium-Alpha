// Intended function: bounded sparse liquid simulation over loaded local cells only.
#pragma once

#include "core/Saturating.hpp"
#include "world/PlanetScale.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace elysium {

enum class FluidKind : std::uint8_t { Water, Lava, Acid };

struct FluidCell {
    LargeSurfaceCellAddress address{};
    FluidKind kind{FluidKind::Water};
    double volume01{};
    double temperatureK{};
    double contamination01{};
};

struct FluidTransfer {
    LargeSurfaceCellAddress from{};
    LargeSurfaceCellAddress to{};
    FluidKind kind{FluidKind::Water};
    double amount01{};
};

struct FluidReaction {
    LargeSurfaceCellAddress cell{};
    FluidKind a{FluidKind::Water};
    FluidKind b{FluidKind::Water};
    double consumedA{};
    double consumedB{};
    std::uint64_t productMaterialKey{};
    double heatDelta{};
};

struct FluidStepResult {
    std::vector<FluidTransfer> transfers;
    std::vector<FluidReaction> reactions;
    double escapedVolume{};
};

class IFluidNeighborhood {
public:
    virtual ~IFluidNeighborhood() = default;
    virtual bool isSolid(const LargeSurfaceCellAddress& cell) const = 0;
    virtual double fluidCapacity01(const LargeSurfaceCellAddress& cell) const = 0;
    virtual std::vector<LargeSurfaceCellAddress> neighbors(const LargeSurfaceCellAddress& cell) const = 0;
};

class BoundedFluidSolver final {
public:
    // Pure compute: caller supplies a bounded snapshot and applies transfers later
    // on the semantic owner phase. No whole-planet search occurs here.
    [[nodiscard]] FluidStepResult compute(
        std::span<const FluidCell> cells,
        const IFluidNeighborhood& neighborhood,
        std::size_t transferBudget) const;
};

} // namespace elysium
