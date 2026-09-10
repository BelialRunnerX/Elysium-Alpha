// Intended function: Project versioned game/accessibility/audio/graphics/input/simulation settings with validation ranges and restart requirements.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ui {
struct SettingsViewState {
    std::uint64_t viewId{};
    std::uint64_t category{};
    std::uint64_t optionCount{};
    std::uint64_t changedCount{};
    std::uint64_t restartRequired{};
    std::uint64_t flags{};
};
class SettingsViewStateCollection {
public:
 bool store(SettingsViewState value); bool erase(std::uint64_t id); [[nodiscard]] const SettingsViewState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<SettingsViewState>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const SettingsViewState& v) noexcept; std::vector<SettingsViewState> rows_;
};
}
