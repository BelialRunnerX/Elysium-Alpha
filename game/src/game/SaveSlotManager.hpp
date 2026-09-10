// Intended function: Track save slots, world seeds, display names, timestamps, schema/generator identity, thumbnails, and compatibility state.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::game {
struct SaveSlotState {
    std::uint64_t slotId{};
    std::uint64_t worldSeed{};
    std::uint64_t schema{};
    std::uint64_t generatorVersion{};
    double timestamp{};
    std::uint64_t flags{};
};
class SaveSlotStateCollection {
public:
 bool store(SaveSlotState value); bool erase(std::uint64_t id); [[nodiscard]] const SaveSlotState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<SaveSlotState>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const SaveSlotState& v) noexcept; std::vector<SaveSlotState> rows_;
};
}
