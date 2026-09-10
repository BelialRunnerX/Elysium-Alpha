// Intended function: Represent radial/local gravity frames, tangent basis identity, gravity strength, and transitions for spherical planetary play.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::physics {
struct GravityFrameState {
    std::uint64_t frameId{};
    std::uint64_t planetId{};
    std::uint64_t positionHash{};
    std::uint64_t upHash{};
    double gravity{};
    std::uint64_t revision{};
};
class GravityFrameStateIndex {
public:
 bool upsert(GravityFrameState value); bool erase(std::uint64_t id); [[nodiscard]] const GravityFrameState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<GravityFrameState>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const GravityFrameState& value) noexcept; std::vector<GravityFrameState> rows_;
};
}
