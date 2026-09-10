// Intended function: Track deterministic space hazard volumes such as radiation belts, debris, storms, anomalies, and interdiction zones.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::travel {
struct SpaceHazard {
    double hazardId{};
    std::uint64_t systemId{};
    std::uint64_t kind{};
    double severity{};
    double radius{};
    std::uint64_t expiresTick{};
};
class SpaceHazardRegistry {
public:
    bool publish(SpaceHazard record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const SpaceHazard* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<SpaceHazard>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const SpaceHazard& r) noexcept;
    std::vector<SpaceHazard> records_;
};
} // namespace elysium::travel
