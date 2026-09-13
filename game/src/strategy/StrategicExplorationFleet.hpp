#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track long-range exploration fleets, supplies, discoveries, route risk, communication delay, and return policy.
struct StrategicExplorationFleetCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct StrategicExplorationFleetState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class StrategicExplorationFleetSystem { public: bool submit(const StrategicExplorationFleetCommand&); const StrategicExplorationFleetState* find(std::uint64_t) const; std::vector<StrategicExplorationFleetState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,StrategicExplorationFleetState> map_; };
}
