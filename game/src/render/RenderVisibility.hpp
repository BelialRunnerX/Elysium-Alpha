// Intended function: imported render implementation for RenderVisibility; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "core/Math.hpp"

#include <cstddef>
#include <cstdint>

namespace elysium {

// Backend-neutral camera description used to build a draw list before any
// graphics API is touched. Positions are expressed in the same planet/world
// coordinate space as mesh packet bounds; renderOrigin is applied only at draw
// publication time for floating-origin precision.
struct RenderView {
    Vec3 position{};
    Vec3 forward{0.0f,0.0f,1.0f};
    Vec3 up{0.0f,1.0f,0.0f};
    float verticalFovDegrees{75.0f};
    float aspect{16.0f/9.0f};
    float nearPlane{0.05f};
    float farPlane{10000.0f};
};

struct RenderBounds {
    Vec3 center{};
    float radius{};
    bool valid{};
};

struct RendererDrawStats {
    int candidateMeshes{};
    int drawnMeshes{};
    int frustumCulled{};
    int horizonCulled{};
    int drawCalls{};
    int triangles{};
};

struct RendererResourceStats {
    std::size_t estimatedGpuBytes{};
    std::uint64_t uploads{};
    std::uint64_t destroys{};
    double lastUploadMs{};
    double maxUploadMs{};
    double lastEditToVisibleMs{};
    double maxEditToVisibleMs{};
};

// Conservative sphere-vs-view-frustum test. A false result means the entire
// packet is outside at least one camera plane; true may still include a small
// amount of overdraw by design.
bool visibleInFrustum(const RenderView& view, const RenderBounds& bounds);

// Planet horizon rejection for a planet centered at planetCenter. This is also
// conservative: a packet is rejected only when its bounding sphere is entirely
// behind the geometric horizon. Cameras at/below the reference radius disable
// horizon rejection because local tunnels/interiors can invalidate a pure shell
// assumption.
bool visibleAbovePlanetHorizon(const RenderView& view,
                               const RenderBounds& bounds,
                               Vec3 planetCenter,
                               float referenceRadius);

} // namespace elysium
