#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track stable drone fleets, roles, batteries, payloads, maintenance, dispatch, recall, and remote aggregate state.
struct DroneFleetSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct DroneFleetSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class DroneFleetSystemSystem { public: bool submit(const DroneFleetSystemCommand&); const DroneFleetSystemState* find(std::uint64_t) const; std::vector<DroneFleetSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,DroneFleetSystemState> map_; };
}
