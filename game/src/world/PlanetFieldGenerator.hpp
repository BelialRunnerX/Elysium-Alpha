// Intended function: seam-continuous spherical climate, height, and 3D density fields sampled on demand.
#pragma once

#include "core/Math.hpp"
#include "world/PlanetFormationProfile.hpp"
#include "world/PlanetScale.hpp"

#include <cstdint>

namespace elysium {

struct PlanetFieldSample {
    double temperature01{};
    double moisture01{};
    double radiation01{};
    double tectonic01{};
    double erosion01{};
    double ridge01{};
    double surfaceHeightMeters{};
    double signedDensity{}; // >0 solid, <=0 empty
};

class PlanetFieldGenerator final {
public:
    PlanetFieldGenerator(std::uint64_t universeSeed, PlanetPhysicalProfile profile, PlanetScaleDescriptor scale);

    [[nodiscard]] const PlanetPhysicalProfile& profile() const noexcept { return profile_; }
    [[nodiscard]] const PlanetScaleDescriptor& scale() const noexcept { return scale_; }

    // `direction` is planet-center-relative and normalized internally. Using a
    // shared 3D direction rather than face-local UV guarantees seam continuity.
    [[nodiscard]] PlanetFieldSample sample(Vec3 direction, double radialOffsetMeters) const noexcept;

private:
    [[nodiscard]] std::uint64_t sub(std::uint64_t domain, std::uint64_t extra = 0) const noexcept;
    [[nodiscard]] double noise3(std::uint64_t seed, double x, double y, double z) const noexcept;
    [[nodiscard]] double fractal(std::uint64_t seed, Vec3 direction, double scale, std::uint32_t octaves) const noexcept;

    std::uint64_t universeSeed_{};
    PlanetPhysicalProfile profile_{};
    PlanetScaleDescriptor scale_{};
};

} // namespace elysium
