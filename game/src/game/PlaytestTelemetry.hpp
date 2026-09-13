// Intended function: Capture vertical-slice observables such as time-to-shelter, crafting milestones, deaths, edit latency, travel motives, and return behavior.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::game {
struct PlaytestSample {
    std::uint64_t sampleId{};
    std::uint64_t metricId{};
    double value{};
    std::uint64_t tick{};
    std::uint64_t contextId{};
    std::uint64_t flags{};
};
class PlaytestSampleCollection {
public:
 bool store(PlaytestSample value); bool erase(std::uint64_t id); [[nodiscard]] const PlaytestSample* find(std::uint64_t id) const; [[nodiscard]] const std::vector<PlaytestSample>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const PlaytestSample& v) noexcept; std::vector<PlaytestSample> rows_;
};
}
