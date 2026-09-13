// Intended function: Accumulate bounded counters/timers for chunking, meshing, ECS phases, logistics, AI, saves, and rendering budgets.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::tools {
struct PerformanceSample {
    std::uint64_t sampleId{};
    std::uint64_t counterId{};
    double value{};
    double budget{};
    std::uint64_t tick{};
    std::uint64_t flags{};
};
class PerformanceSampleRegistry {
public:
    bool publish(PerformanceSample record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const PerformanceSample* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<PerformanceSample>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const PerformanceSample& r) noexcept;
    std::vector<PerformanceSample> records_;
};
} // namespace elysium::tools
