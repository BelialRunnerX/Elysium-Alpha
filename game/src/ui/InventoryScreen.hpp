// Intended function: Project player/container inventories, filters, stacks, stable items, equipment, hotbar, encumbrance, and transfer actions.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ui {
struct InventoryViewState {
    std::uint64_t viewId{};
    std::uint64_t ownerId{};
    std::uint64_t containerId{};
    std::uint64_t itemCount{};
    double capacity{};
    std::uint64_t flags{};
};
class InventoryViewStateCollection {
public:
 bool store(InventoryViewState value); bool erase(std::uint64_t id); [[nodiscard]] const InventoryViewState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<InventoryViewState>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const InventoryViewState& v) noexcept; std::vector<InventoryViewState> rows_;
};
}
