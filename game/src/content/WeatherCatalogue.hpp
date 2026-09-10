// Intended function: Provide stable weather definitions, transitions, hazard multipliers, visibility, precipitation, and VFX/audio tags.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::content {
struct WeatherRecord {
    std::uint64_t weatherId{};
    double hazardClass{};
    double temperatureDelta{};
    std::uint64_t visibility{};
    std::uint64_t precipitation{};
    std::uint64_t effectTag{};
};
class WeatherRecordRegistry {
public:
    bool publish(WeatherRecord record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const WeatherRecord* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<WeatherRecord>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const WeatherRecord& r) noexcept;
    std::vector<WeatherRecord> records_;
};
} // namespace elysium::content
