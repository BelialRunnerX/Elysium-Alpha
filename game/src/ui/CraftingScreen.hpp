// Intended function: Project recipes, stations, inputs, queues, quality, unlocks, blockers, and batch controls for embodied/fortress crafting.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ui {
struct CraftingViewState {
    std::uint64_t viewId{};
    std::uint64_t stationId{};
    std::uint64_t recipeId{};
    std::uint64_t queueCount{};
    double progress{};
    std::uint64_t flags{};
};
class CraftingViewStateCollection {
public:
 bool store(CraftingViewState value); bool erase(std::uint64_t id); [[nodiscard]] const CraftingViewState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<CraftingViewState>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const CraftingViewState& v) noexcept; std::vector<CraftingViewState> rows_;
};
}
