// Intended function: Sample shared 3D planetary climate fields for temperature, humidity, continentalness, wind, storm energy, and season.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::climate {
struct ClimateSample {
    std::uint64_t sampleId{};
    double temperature{};
    double humidity{};
    double continentalness{};
    double wind{};
    double stormEnergy{};
};
class ClimateSampleStore {
public:
 bool put(ClimateSample v); bool erase(std::uint64_t id);
 [[nodiscard]] const ClimateSample* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<ClimateSample>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const ClimateSample& v) noexcept; std::vector<ClimateSample> values_;
};
} // namespace elysium::climate
