// Intended function: local proto-chunk+halo sampling over a huge sparse cube-sphere address space.
#include "world/ProtoChunkGenerator.hpp"

#include "core/Saturating.hpp"
#include "world/CubeSphere.hpp"

#include <algorithm>
#include <limits>

namespace elysium {

Vec3 ProtoChunkGenerator::directionFor(CubeFace face, std::uint64_t u, std::uint64_t v) const {
    const double u01 = PlanetScale::faceCoordinate01(scale_, u);
    const double v01 = PlanetScale::faceCoordinate01(scale_, v);
    return faceUvToDirection(face,
        static_cast<float>(u01 * 2.0 - 1.0),
        static_cast<float>(v01 * 2.0 - 1.0));
}

ProtoChunk ProtoChunkGenerator::generate(
    LargePlanetChunkAddress address,
    std::uint32_t edge,
    std::uint32_t halo) const {
    edge = std::clamp<std::uint32_t>(edge, 1U, 128U);
    halo = std::min<std::uint32_t>(halo, 8U);
    address.planetId = scale_.planetId;
    ProtoChunk out{.address = address, .edge = edge, .halo = halo};
    const std::uint64_t width = static_cast<std::uint64_t>(edge) + static_cast<std::uint64_t>(halo) * 2ULL;
    const auto plane = safe::saturatingMultiply<std::uint64_t>(width, width);
    const auto count = safe::saturatingMultiply<std::uint64_t>(plane, width);
    if (count <= 4'000'000ULL) out.samples.reserve(static_cast<std::size_t>(count));

    const std::uint64_t baseU = safe::saturatingMultiply<std::uint64_t>(address.chunkU, edge);
    const std::uint64_t baseV = safe::saturatingMultiply<std::uint64_t>(address.chunkV, edge);
    const std::int64_t baseR = address.chunkRadial > std::numeric_limits<std::int64_t>::max() / static_cast<std::int64_t>(edge)
        ? std::numeric_limits<std::int64_t>::max()
        : address.chunkRadial < std::numeric_limits<std::int64_t>::lowest() / static_cast<std::int64_t>(edge)
            ? std::numeric_limits<std::int64_t>::lowest()
            : address.chunkRadial * static_cast<std::int64_t>(edge);

    for (std::int64_t rz = -static_cast<std::int64_t>(halo); rz < static_cast<std::int64_t>(edge + halo); ++rz) {
        for (std::int64_t vy = -static_cast<std::int64_t>(halo); vy < static_cast<std::int64_t>(edge + halo); ++vy) {
            for (std::int64_t ux = -static_cast<std::int64_t>(halo); ux < static_cast<std::int64_t>(edge + halo); ++ux) {
                const std::uint64_t u = ux < 0 && static_cast<std::uint64_t>(-ux) > baseU ? 0ULL : static_cast<std::uint64_t>(static_cast<std::int64_t>(baseU) + ux);
                const std::uint64_t v = vy < 0 && static_cast<std::uint64_t>(-vy) > baseV ? 0ULL : static_cast<std::uint64_t>(static_cast<std::int64_t>(baseV) + vy);
                const std::int64_t radial = baseR + rz;
                auto cell = PlanetScale::clampToFace(scale_, {scale_.planetId, address.face, u, v, radial});
                const auto dir = directionFor(cell.face, cell.u, cell.v);
                const double radialMeters = static_cast<double>(cell.radial) * scale_.macroCellMeters;
                const auto sample = fields_->sample(dir, radialMeters);
                out.samples.push_back({
                    .address = cell,
                    .direction = dir,
                    .radialOffsetMeters = radialMeters,
                    .density = sample.signedDensity,
                    .surfaceHeightMeters = sample.surfaceHeightMeters,
                });
            }
        }
    }
    return out;
}

} // namespace elysium
