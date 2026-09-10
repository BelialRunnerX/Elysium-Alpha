// Intended function: Coordinate loading/streaming transitions between sites, planets, orbit, systems, retire/reclaim, and save reload boundaries.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::game {
struct TransitionState {
    std::uint64_t transitionId{};
    std::uint64_t fromSite{};
    std::uint64_t toSite{};
    std::uint64_t phase{};
    double progress{};
    std::uint64_t flags{};
};
class TransitionStateCollection {
public:
 bool store(TransitionState value); bool erase(std::uint64_t id); [[nodiscard]] const TransitionState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<TransitionState>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const TransitionState& v) noexcept; std::vector<TransitionState> rows_;
};
}
