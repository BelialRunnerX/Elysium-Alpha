// Intended function: Provide stable planet-class descriptors with gravity, atmosphere, thermal, radiation, biome, and resource biases.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::content {
struct PlanetClassRecord {
    std::uint64_t planetClassId{};
    double gravity{};
    double pressure{};
    double temperature{};
    double radiation{};
    double resourceBias{};
};
class PlanetClassRecordRegistry {
public:
    bool publish(PlanetClassRecord record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const PlanetClassRecord* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<PlanetClassRecord>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const PlanetClassRecord& r) noexcept;
    std::vector<PlanetClassRecord> records_;
};
} // namespace elysium::content
