#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate system/planet defense posture from threats, sensors, fleets, fortifications, logistics, and evacuation capacity.
struct DefensePlannerCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct DefensePlannerState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class DefensePlannerSystem { public: bool submit(const DefensePlannerCommand&); const DefensePlannerState* find(std::uint64_t) const; std::vector<DefensePlannerState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,DefensePlannerState> map_; };
}
