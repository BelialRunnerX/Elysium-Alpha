// Intended function: derive huge sparse world-address envelopes from physical planet formation results.
#include "world/PlanetScale.hpp"

#include "core/Saturating.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace elysium {
namespace {
constexpr double Pi = 3.14159265358979323846;

std::int64_t floorDiv(std::int64_t value, std::int64_t divisor) noexcept {
    if (divisor <= 0) return 0;
    const auto q = value / divisor;
    const auto r = value % divisor;
    return r != 0 && ((r < 0) != (divisor < 0)) ? q - 1 : q;
}
} // namespace

PlanetScaleDescriptor PlanetScale::fromFormation(
    PlanetStableId planetId,
    const PlanetFormationSample& formation,
    double macroCellMeters) noexcept {
    const double cell = safe::nonNegative(macroCellMeters, 1.0e6);
    const double usableCell = std::max(0.01, cell);
    const double radiusMeters = safe::nonNegative(formation.radiusKm * 1000.0);
    const double circumference = safe::nonNegative(2.0 * Pi * radiusMeters);
    // One cube-sphere face spans 90 degrees along a great circle: pi*R/2.
    const double faceArc = safe::nonNegative(0.5 * Pi * radiusMeters);
    const double columnsD = safe::nonNegative(std::ceil(faceArc / usableCell));
    const auto columns = safe::saturatingCast<std::uint64_t>(
        static_cast<std::uint64_t>(std::min(columnsD, static_cast<double>(std::numeric_limits<std::uint64_t>::max()))));
    const std::uint64_t faceColumns = std::max<std::uint64_t>(1ULL, columns);
    const auto faceArea = safe::saturatingMultiply<std::uint64_t>(faceColumns, faceColumns);
    const auto allFaces = safe::saturatingMultiply<std::uint64_t>(faceArea, 6ULL);
    return {
        .planetId = planetId,
        .radiusMeters = radiusMeters,
        .circumferenceMeters = circumference,
        .macroCellMeters = usableCell,
        .faceColumns = faceColumns,
        .surfaceColumnEstimate = allFaces,
    };
}

LargePlanetChunkAddress PlanetScale::chunkOf(
    const PlanetScaleDescriptor& scale,
    LargeSurfaceCellAddress address,
    std::uint64_t chunkEdge) noexcept {
    const auto edge = std::max<std::uint64_t>(1ULL, chunkEdge);
    address = clampToFace(scale, address);
    const auto signedEdge = edge > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())
        ? std::numeric_limits<std::int64_t>::max()
        : static_cast<std::int64_t>(edge);
    return {
        .planetId = scale.planetId,
        .face = address.face,
        .chunkU = address.u / edge,
        .chunkV = address.v / edge,
        .chunkRadial = floorDiv(address.radial, signedEdge),
    };
}

LargeSurfaceCellAddress PlanetScale::clampToFace(
    const PlanetScaleDescriptor& scale,
    LargeSurfaceCellAddress address) noexcept {
    address.planetId = scale.planetId;
    const std::uint64_t maxColumn = scale.faceColumns == 0 ? std::uint64_t{0} : static_cast<std::uint64_t>(scale.faceColumns - std::uint64_t{1});
    address.u = std::min<std::uint64_t>(address.u, maxColumn);
    address.v = std::min<std::uint64_t>(address.v, maxColumn);
    return address;
}

double PlanetScale::faceCoordinate01(
    const PlanetScaleDescriptor& scale,
    std::uint64_t column) noexcept {
    if (scale.faceColumns <= 1) return 0.5;
    const std::uint64_t c = std::min<std::uint64_t>(column, static_cast<std::uint64_t>(scale.faceColumns - std::uint64_t{1}));
    return safe::finiteClamp(
        static_cast<double>(c) / static_cast<double>(scale.faceColumns - 1ULL), 0.0, 1.0);
}

} // namespace elysium
