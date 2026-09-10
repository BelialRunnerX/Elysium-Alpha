// Intended function: imported render implementation for DetailInstances; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "render/GraphicsBackend.hpp"
#include "render/RenderVisibility.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace elysium {

// Regenerable decorative/detail geometry is intentionally separate from voxel
// persistence. A cluster contains stable, seed-derived instance descriptors;
// only material edits/destruction need to become persistent world state.
struct DetailInstance {
    std::uint64_t stableKey{};
    GraphicsInstance transform{};
    float importance{1.0f};
    float maxDistance{96.0f};
    int triangleCost{};
    // Conservative local-space bounding sphere used only for presentation
    // culling. This is regenerated content metadata and is never save state.
    float boundsRadius{0.5f};
};

struct DetailInstanceBudget {
    int maxInstances{4096};
    int maxTriangles{200000};
    float maxDistance{128.0f};
};

struct DetailInstanceStats {
    int considered{};
    int selected{};
    int triangles{};
    int distanceRejected{};
    int frustumRejected{};
    int horizonRejected{};
    int budgetRejected{};
};

struct DetailInstanceSelection {
    std::vector<GraphicsInstance> instances;
    DetailInstanceStats stats{};
};

// Stable selection independent of input/container order. Higher importance wins,
// then nearer instances, then stableKey. This lets detail density degrade under
// explicit budgets without changing authoritative voxel meaning.
DetailInstanceSelection selectDetailInstances(const std::vector<DetailInstance>& candidates,
                                              Vec3 cameraPosition,
                                              const DetailInstanceBudget& budget);


// Visibility-aware deterministic selection. Frustum rejection is always
// applied. Planet horizon rejection is enabled only when
// planetReferenceRadius > 0, keeping the same API usable for planar/interior
// detail. Rejected candidates never consume instance/triangle budget.
DetailInstanceSelection selectVisibleDetailInstances(const std::vector<DetailInstance>& candidates,
                                                     const RenderView& view,
                                                     const DetailInstanceBudget& budget,
                                                     Vec3 planetCenter = {},
                                                     float planetReferenceRadius = 0.0f);

// Backend-neutral batched draw. The backend may implement actual hardware
// instancing or conservatively expand the batch; callers get one semantic batch
// either way and can account for its instance/triangle cost.
void drawDetailInstances(IGraphicsBackend& backend,
                         GraphicsMeshHandle mesh,
                         const DetailInstanceSelection& selection,
                         Vec3 renderOrigin = {});

} // namespace elysium
