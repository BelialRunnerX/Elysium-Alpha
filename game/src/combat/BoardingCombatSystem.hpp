#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Coordinate room-to-room boarding objectives, squads, doors, atmosphere, civilians, capture, sabotage, and retreat.
struct BoardingCombatSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct BoardingCombatSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class BoardingCombatSystemSystem { public: bool submit(const BoardingCombatSystemCommand&); const BoardingCombatSystemState* find(std::uint64_t) const; std::vector<BoardingCombatSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,BoardingCombatSystemState> map_; };
}
