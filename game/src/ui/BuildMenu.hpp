// Intended function: Project available structures/blocks/blueprints, material costs, blockers, power/atmosphere requirements, and placement modes.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ui {
struct BuildMenuState {
    std::uint64_t viewId{};
    std::uint64_t category{};
    std::uint64_t selectionId{};
    std::uint64_t availableCount{};
    std::uint64_t blockerCount{};
    std::uint64_t flags{};
};
class BuildMenuStateCollection {
public:
 bool store(BuildMenuState value); bool erase(std::uint64_t id); [[nodiscard]] const BuildMenuState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<BuildMenuState>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const BuildMenuState& v) noexcept; std::vector<BuildMenuState> rows_;
};
}
