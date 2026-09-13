// Intended function: Represent orbital mining targets, extraction progress, yield grade, hazard exposure, and cargo destination.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::travel {
struct OrbitalMiningJob {
    std::uint64_t jobId{};
    std::uint64_t targetId{};
    std::uint64_t progress{};
    double yieldGrade{};
    double hazard{};
    std::uint64_t destinationId{};
};
class OrbitalMiningJobRegistry {
public:
    bool publish(OrbitalMiningJob record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const OrbitalMiningJob* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<OrbitalMiningJob>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const OrbitalMiningJob& r) noexcept;
    std::vector<OrbitalMiningJob> records_;
};
} // namespace elysium::travel
