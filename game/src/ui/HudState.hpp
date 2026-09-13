// Intended function: Project authoritative player vitals, equipment, hotbar, hazards, alerts, objectives, and interaction prompts into HUD state.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::ui {
struct HudSnapshot {
    std::uint64_t playerId{};
    double health{};
    double oxygen{};
    double energy{};
    std::uint64_t alertCount{};
    std::uint64_t objectiveId{};
};
class HudSnapshotRegistry {
public:
    bool publish(HudSnapshot record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const HudSnapshot* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<HudSnapshot>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const HudSnapshot& r) noexcept;
    std::vector<HudSnapshot> records_;
};
} // namespace elysium::ui
