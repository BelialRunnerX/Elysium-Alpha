// Intended function: Track desired movement, grounded state, jump/fly mode, velocity, collision response, slope limits, and fall-damage inputs.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::physics {
struct CharacterMotorState {
    std::uint64_t actorId{};
    std::uint64_t moveHash{};
    std::uint64_t velocityHash{};
    std::uint64_t grounded{};
    std::uint64_t mode{};
    double fallSpeed{};
};
class CharacterMotorStateIndex {
public:
 bool upsert(CharacterMotorState value); bool erase(std::uint64_t id); [[nodiscard]] const CharacterMotorState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<CharacterMotorState>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const CharacterMotorState& value) noexcept; std::vector<CharacterMotorState> rows_;
};
}
