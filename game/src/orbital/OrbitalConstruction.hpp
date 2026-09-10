// Intended function: Track orbital blueprint placement, material delivery, EVA/robotic work, structural progress, power, and pressurization.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::orbital {
struct OrbitalBuildJob {
    std::uint64_t jobId{};
    std::uint64_t structureId{};
    std::uint64_t orbitId{};
    double progress{};
    std::uint64_t materialsDelivered{};
    std::uint64_t state{};
};
class OrbitalBuildJobStore {
public:
 bool put(OrbitalBuildJob v); bool erase(std::uint64_t id);
 [[nodiscard]] const OrbitalBuildJob* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<OrbitalBuildJob>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const OrbitalBuildJob& v) noexcept; std::vector<OrbitalBuildJob> values_;
};
} // namespace elysium::orbital
