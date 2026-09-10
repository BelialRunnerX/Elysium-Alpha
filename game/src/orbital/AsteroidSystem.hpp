// Intended function: Generate stable asteroid descriptors, composition, mass, rotation, resource veins, hazards, and mining depletion.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::orbital {
struct AsteroidState {
    std::uint64_t asteroidId{};
    std::uint64_t compositionHash{};
    double mass{};
    double rotation{};
    double resourceGrade{};
    double remainingMass{};
};
class AsteroidStateStore {
public:
 bool put(AsteroidState v); bool erase(std::uint64_t id);
 [[nodiscard]] const AsteroidState* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<AsteroidState>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const AsteroidState& v) noexcept; std::vector<AsteroidState> values_;
};
} // namespace elysium::orbital
