// Intended function: imported render implementation for RenderVisibility; preserves the agent-authored subsystem contract for later integration/debugging.
#include "render/RenderVisibility.hpp"

#include <algorithm>
#include <cmath>

namespace elysium {
namespace {

constexpr float kPi = 3.14159265358979323846f;

Vec3 safeNormalized(Vec3 value, Vec3 fallback) {
    return lengthSq(value) > 1.0e-10f ? normalize(value) : fallback;
}

} // namespace

bool visibleInFrustum(const RenderView& view, const RenderBounds& bounds) {
    if (!bounds.valid || bounds.radius < 0.0f) return true;

    const Vec3 forward = safeNormalized(view.forward,{0,0,1});
    Vec3 right = cross(forward,safeNormalized(view.up,{0,1,0}));
    if (lengthSq(right) < 1.0e-8f) right = cross(forward,{1,0,0});
    right = safeNormalized(right,{1,0,0});
    const Vec3 up = safeNormalized(cross(right,forward),{0,1,0});
    const Vec3 to = bounds.center - view.position;
    const float depth = dot(to,forward);
    const float radius = std::max(0.0f,bounds.radius);
    const float nearPlane = std::max(0.0f,view.nearPlane);
    const float farPlane = std::max(nearPlane,view.farPlane);

    if (depth + radius < nearPlane || depth - radius > farPlane) return false;

    const float fov = std::clamp(view.verticalFovDegrees,1.0f,179.0f) * (kPi/180.0f);
    const float tanV = std::tan(fov*0.5f);
    const float tanH = tanV * std::max(0.01f,view.aspect);
    const float y = std::abs(dot(to,up));
    const float x = std::abs(dot(to,right));

    // Distance from a sphere center to each side plane is represented without
    // materializing a matrix. sqrt(1+tan^2) converts the radius to the same
    // unnormalized plane scale.
    if (y > depth*tanV + radius*std::sqrt(1.0f+tanV*tanV)) return false;
    if (x > depth*tanH + radius*std::sqrt(1.0f+tanH*tanH)) return false;
    return true;
}

bool visibleAbovePlanetHorizon(const RenderView& view,
                               const RenderBounds& bounds,
                               Vec3 planetCenter,
                               float referenceRadius) {
    if (!bounds.valid || referenceRadius <= 0.0f) return true;
    const Vec3 camera = view.position - planetCenter;
    const float cameraDistance = length(camera);
    if (cameraDistance <= referenceRadius + 0.01f) return true;

    const Vec3 packet = bounds.center - planetCenter;
    const float packetDistance = length(packet);
    if (packetDistance <= 1.0e-5f) return true;

    const Vec3 cameraDir = camera / cameraDistance;
    const Vec3 packetDir = packet / packetDistance;
    const float horizonAngle = std::acos(std::clamp(referenceRadius/cameraDistance,0.0f,1.0f));
    const float angularRadius = std::asin(std::clamp(std::max(0.0f,bounds.radius)/packetDistance,0.0f,1.0f));
    const float visibleAngle = std::min(kPi,horizonAngle+angularRadius);
    return dot(cameraDir,packetDir) >= std::cos(visibleAngle);
}

} // namespace elysium
