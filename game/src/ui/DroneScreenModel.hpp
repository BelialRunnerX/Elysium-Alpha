#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build drone fleet roles, battery, payload, assignments, maintenance, failures, and automation command projections.
struct DroneScreenModelCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct DroneScreenModelState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class DroneScreenModelSystem { public: bool submit(const DroneScreenModelCommand&); const DroneScreenModelState* find(std::uint64_t) const; std::vector<DroneScreenModelState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,DroneScreenModelState> map_; };
}
