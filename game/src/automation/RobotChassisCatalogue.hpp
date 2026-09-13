#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Register robot chassis roles, mobility, payload, armor, power, sensors, tool slots, and fabrication requirements.
struct RobotChassisCatalogueCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct RobotChassisCatalogueState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class RobotChassisCatalogueSystem { public: bool submit(const RobotChassisCatalogueCommand&); const RobotChassisCatalogueState* find(std::uint64_t) const; std::vector<RobotChassisCatalogueState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,RobotChassisCatalogueState> map_; };
}
