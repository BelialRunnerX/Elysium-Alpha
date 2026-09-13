// Intended function: Manage visitors, merchants, envoys, scholars, mercenaries, guests, hospitality, access, and departure.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::fortress {
struct VisitorState {
    std::uint64_t visitorId{};
    std::uint64_t factionId{};
    std::uint64_t purpose{};
    std::uint64_t arrivalTick{};
    std::uint64_t departureTick{};
    std::uint64_t status{};
};
class VisitorStateStore {
public:
 bool put(VisitorState v); bool erase(std::uint64_t id);
 [[nodiscard]] const VisitorState* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<VisitorState>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const VisitorState& v) noexcept; std::vector<VisitorState> values_;
};
} // namespace elysium::fortress
