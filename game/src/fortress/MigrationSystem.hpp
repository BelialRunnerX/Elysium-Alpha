// Intended function: Evaluate migration waves from safety, wealth, housing, jobs, culture, faction relations, and historical events.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::fortress {
struct MigrationWave {
    std::uint64_t waveId{};
    std::uint64_t originId{};
    std::uint64_t destinationId{};
    double population{};
    std::uint64_t attraction{};
    double risk{};
};
class MigrationWaveStore {
public:
 bool put(MigrationWave v); bool erase(std::uint64_t id);
 [[nodiscard]] const MigrationWave* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<MigrationWave>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const MigrationWave& v) noexcept; std::vector<MigrationWave> values_;
};
} // namespace elysium::fortress
