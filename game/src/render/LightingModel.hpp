// Intended function: Represent sun, sky, local lights, emissive voxels, shadow budgets, and atmosphere-lighting inputs.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::render {
struct LightRecord {
    std::uint64_t lightId{};
    std::uint64_t kind{};
    double intensity{};
    double range{};
    std::uint64_t shadowPriority{};
    std::uint64_t flags{};
};
class LightRecordRegistry {
public:
    bool publish(LightRecord record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const LightRecord* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<LightRecord>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const LightRecord& r) noexcept;
    std::vector<LightRecord> records_;
};
} // namespace elysium::render
