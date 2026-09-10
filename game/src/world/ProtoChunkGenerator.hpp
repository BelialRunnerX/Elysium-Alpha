// Intended function: generate only requested planet chunks plus a small halo from pure fields.
#pragma once

#include "core/Math.hpp"
#include "world/PlanetFieldGenerator.hpp"
#include "world/PlanetResourceField.hpp"
#include "world/PlanetScale.hpp"

#include <cstdint>
#include <vector>

namespace elysium {

struct ProtoVoxelSample {
    LargeSurfaceCellAddress address{};
    Vec3 direction{};
    double radialOffsetMeters{};
    double density{};
    double surfaceHeightMeters{};
};

struct ProtoChunk {
    LargePlanetChunkAddress address{};
    std::uint32_t edge{};
    std::uint32_t halo{};
    std::vector<ProtoVoxelSample> samples;
};

class ProtoChunkGenerator final {
public:
    ProtoChunkGenerator(const PlanetFieldGenerator& fields, PlanetScaleDescriptor scale)
        : fields_(&fields), scale_(scale) {}

    [[nodiscard]] ProtoChunk generate(
        LargePlanetChunkAddress address,
        std::uint32_t edge = 32,
        std::uint32_t halo = 1) const;

private:
    [[nodiscard]] Vec3 directionFor(CubeFace face, std::uint64_t u, std::uint64_t v) const;

    const PlanetFieldGenerator* fields_{};
    PlanetScaleDescriptor scale_{};
};

} // namespace elysium
