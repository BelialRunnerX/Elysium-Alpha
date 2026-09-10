// Intended function: imported render implementation for DetailInstances; preserves the agent-authored subsystem contract for later integration/debugging.
#include "render/DetailInstances.hpp"

#include <algorithm>
#include <cmath>

namespace elysium {

namespace {

struct RankedDetail { const DetailInstance* instance{}; float distanceSq{}; };

DetailInstanceSelection finalizeSelection(std::vector<RankedDetail> ranked,
                                        DetailInstanceSelection out,
                                        const DetailInstanceBudget& budget) {
    std::sort(ranked.begin(),ranked.end(),[](const RankedDetail& a,const RankedDetail& b) {
        if (a.instance->importance!=b.instance->importance) return a.instance->importance>b.instance->importance;
        if (a.distanceSq!=b.distanceSq) return a.distanceSq<b.distanceSq;
        return a.instance->stableKey<b.instance->stableKey;
    });

    const int maxInstances=std::max(0,budget.maxInstances);
    const int maxTriangles=std::max(0,budget.maxTriangles);
    out.instances.reserve(static_cast<std::size_t>(std::min(maxInstances,static_cast<int>(ranked.size()))));
    for (const auto& rankedInstance:ranked) {
        const auto& candidate=*rankedInstance.instance;
        const int cost=std::max(0,candidate.triangleCost);
        if (out.stats.selected>=maxInstances || out.stats.triangles+cost>maxTriangles) {
            ++out.stats.budgetRejected;
            continue;
        }
        out.instances.push_back(candidate.transform);
        ++out.stats.selected;
        out.stats.triangles+=cost;
    }
    return out;
}

} // namespace

DetailInstanceSelection selectDetailInstances(const std::vector<DetailInstance>& candidates,
                                              Vec3 cameraPosition,
                                              const DetailInstanceBudget& budget) {
    std::vector<RankedDetail> ranked;
    ranked.reserve(candidates.size());
    DetailInstanceSelection out;
    out.stats.considered=static_cast<int>(candidates.size());
    const float globalDistance=std::max(0.0f,budget.maxDistance);
    for (const auto& candidate:candidates) {
        const float distanceSq=lengthSq(candidate.transform.translation-cameraPosition);
        const float allowed=std::max(0.0f,std::min(globalDistance,candidate.maxDistance));
        if (distanceSq>allowed*allowed) { ++out.stats.distanceRejected; continue; }
        ranked.push_back({&candidate,distanceSq});
    }
    return finalizeSelection(std::move(ranked),std::move(out),budget);
}

DetailInstanceSelection selectVisibleDetailInstances(const std::vector<DetailInstance>& candidates,
                                                     const RenderView& view,
                                                     const DetailInstanceBudget& budget,
                                                     Vec3 planetCenter,
                                                     float planetReferenceRadius) {
    std::vector<RankedDetail> ranked;
    ranked.reserve(candidates.size());
    DetailInstanceSelection out;
    out.stats.considered=static_cast<int>(candidates.size());
    const float globalDistance=std::max(0.0f,budget.maxDistance);
    for (const auto& candidate:candidates) {
        const float distanceSq=lengthSq(candidate.transform.translation-view.position);
        const float allowed=std::max(0.0f,std::min(globalDistance,candidate.maxDistance));
        if (distanceSq>allowed*allowed) { ++out.stats.distanceRejected; continue; }

        const RenderBounds bounds{candidate.transform.translation,
                                  std::max(0.0f,candidate.boundsRadius*candidate.transform.uniformScale),
                                  true};
        if (!visibleInFrustum(view,bounds)) { ++out.stats.frustumRejected; continue; }
        if (planetReferenceRadius>0.0f &&
            !visibleAbovePlanetHorizon(view,bounds,planetCenter,planetReferenceRadius)) {
            ++out.stats.horizonRejected;
            continue;
        }
        ranked.push_back({&candidate,distanceSq});
    }
    return finalizeSelection(std::move(ranked),std::move(out),budget);
}

void drawDetailInstances(IGraphicsBackend& backend,
                         GraphicsMeshHandle mesh,
                         const DetailInstanceSelection& selection,
                         Vec3 renderOrigin) {
    if (!mesh || selection.instances.empty()) return;
    backend.drawMeshInstances(mesh,selection.instances,renderOrigin);
}

} // namespace elysium
