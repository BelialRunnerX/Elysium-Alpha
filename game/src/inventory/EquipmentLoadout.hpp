// Intended function: Track operative worn gear/trinkets/tools/weapons, slot rules, layers, condition, encumbrance, and derived readiness.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::inventory {
struct LoadoutState {
    std::uint64_t loadoutId{};
    std::uint64_t ownerId{};
    std::uint64_t slotHash{};
    double mass{};
    double readiness{};
    std::uint64_t flags{};
};
class LoadoutStateIndex {
public:
 bool upsert(LoadoutState value); bool erase(std::uint64_t id); [[nodiscard]] const LoadoutState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<LoadoutState>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const LoadoutState& value) noexcept; std::vector<LoadoutState> rows_;
};
}
