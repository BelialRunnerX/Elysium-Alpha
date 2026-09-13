#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate escort, patrol, intercept, focus-fire, suppress, retreat, recharge, and repair tactical intents for drones.
struct CombatDroneAICommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct CombatDroneAIState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class CombatDroneAISystem { public: bool submit(const CombatDroneAICommand&); const CombatDroneAIState* find(std::uint64_t) const; std::vector<CombatDroneAIState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CombatDroneAIState> map_; };
}
