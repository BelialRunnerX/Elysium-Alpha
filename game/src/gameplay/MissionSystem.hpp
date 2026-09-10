// Intended function: Track multi-stage objectives, optional branches, timers, rewards, and Chronicle-significant mission outcomes.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::gameplay {

struct MissionState {
    std::uint64_t missionId{};
    std::uint64_t stage{};
    double progress{};
    std::uint64_t target{};
    std::uint64_t deadlineTick{};
    std::uint64_t flags{};
};

class MissionStateStore {
public:
    bool upsert(MissionState value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const MissionState* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<MissionState> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const MissionState& value) noexcept;
    std::vector<MissionState> records_;
};

} // namespace elysium::gameplay
