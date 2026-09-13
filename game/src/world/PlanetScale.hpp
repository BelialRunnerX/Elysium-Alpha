// Intended function: scalable cube-sphere addressing for very large planets without dense whole-world allocation.
#pragma once

#include "galaxy/UniverseFormationFields.hpp"
#include "world/CubeSphere.hpp"

#include <cstdint>

namespace elysium {

using PlanetStableId = std::uint64_t;

struct PlanetScaleDescriptor {
    PlanetStableId planetId{};
    double radiusMeters{};
    double circumferenceMeters{};
    double macroCellMeters{1.0};
    std::uint64_t faceColumns{}; // columns along one 90-degree face edge
    std::uint64_t surfaceColumnEstimate{};
};

struct LargeSurfaceCellAddress {
    PlanetStableId planetId{};
    CubeFace face{CubeFace::PositiveZ};
    std::uint64_t u{};
    std::uint64_t v{};
    std::int64_t radial{}; // signed metres/cells relative to reference surface

    auto operator<=>(const LargeSurfaceCellAddress&) const = default;
};

struct LargePlanetChunkAddress {
    PlanetStableId planetId{};
    CubeFace face{CubeFace::PositiveZ};
    std::uint64_t chunkU{};
    std::uint64_t chunkV{};
    std::int64_t chunkRadial{};

    auto operator<=>(const LargePlanetChunkAddress&) const = default;
};

class PlanetScale final {
public:
    static constexpr std::uint64_t DefaultChunkEdge = 32ULL;

    [[nodiscard]] static PlanetScaleDescriptor fromFormation(
        PlanetStableId planetId,
        const PlanetFormationSample& formation,
        double macroCellMeters = 1.0) noexcept;

    [[nodiscard]] static LargePlanetChunkAddress chunkOf(
        const PlanetScaleDescriptor& scale,
        LargeSurfaceCellAddress address,
        std::uint64_t chunkEdge = DefaultChunkEdge) noexcept;

    [[nodiscard]] static LargeSurfaceCellAddress clampToFace(
        const PlanetScaleDescriptor& scale,
        LargeSurfaceCellAddress address) noexcept;

    [[nodiscard]] static double faceCoordinate01(
        const PlanetScaleDescriptor& scale,
        std::uint64_t column) noexcept;
};

} // namespace elysium
