// Intended function: Track player hotbar slot bindings, active slot, content/item stable identity, quick-use cooldown, and persistence state.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::inventory {
struct HotbarSlot {
    std::uint64_t slotId{};
    std::uint64_t itemStableId{};
    std::uint64_t contentId{};
    double quantity{};
    double cooldown{};
    std::uint64_t flags{};
};
class HotbarSlotIndex {
public:
 bool upsert(HotbarSlot value); bool erase(std::uint64_t id); [[nodiscard]] const HotbarSlot* find(std::uint64_t id) const; [[nodiscard]] const std::vector<HotbarSlot>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const HotbarSlot& value) noexcept; std::vector<HotbarSlot> rows_;
};
}
