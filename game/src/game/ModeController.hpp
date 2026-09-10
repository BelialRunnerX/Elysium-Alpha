// Intended function: Coordinate Direct Operative, Fortress Command, Tactical Command, Star-System Strategy, and Chronicle mode transitions.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::game {
struct ModeState {
    std::uint64_t sessionId{};
    std::uint64_t currentMode{};
    std::uint64_t previousMode{};
    std::uint64_t focusStableId{};
    std::uint64_t transitionTick{};
    std::uint64_t flags{};
};
class ModeStateCollection {
public:
 bool store(ModeState value); bool erase(std::uint64_t id); [[nodiscard]] const ModeState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<ModeState>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const ModeState& v) noexcept; std::vector<ModeState> rows_;
};
}
