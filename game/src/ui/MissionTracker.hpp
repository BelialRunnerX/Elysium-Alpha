// Intended function: Project active mission/objective stages, progress, optional goals, timers, target markers, blockers, and rewards.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ui {
struct MissionTrackerState {
    std::uint64_t viewId{};
    std::uint64_t missionId{};
    std::uint64_t objectiveId{};
    double progress{};
    std::uint64_t deadlineTick{};
    std::uint64_t flags{};
};
class MissionTrackerStateCollection {
public:
 bool store(MissionTrackerState value); bool erase(std::uint64_t id); [[nodiscard]] const MissionTrackerState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<MissionTrackerState>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const MissionTrackerState& v) noexcept; std::vector<MissionTrackerState> rows_;
};
}
