// Intended function: Plan local wildlife roaming, feeding, fleeing, hunting, nesting, territorial behavior, and active-bubble demotion.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::wildlife {
struct WildlifeState {
    std::uint64_t actorId{};
    std::uint64_t speciesId{};
    std::uint64_t behavior{};
    double energy{};
    double threat{};
    double targetId{};
};
class WildlifeStateStore {
public:
 bool put(WildlifeState v); bool erase(std::uint64_t id);
 [[nodiscard]] const WildlifeState* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<WildlifeState>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const WildlifeState& v) noexcept; std::vector<WildlifeState> values_;
};
} // namespace elysium::wildlife
