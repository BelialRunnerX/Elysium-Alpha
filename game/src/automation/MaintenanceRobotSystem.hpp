#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track maintenance robots, toolkits, spare parts, charging, assigned facilities, and work queues.
struct MaintenanceRobotSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct MaintenanceRobotSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class MaintenanceRobotSystemSystem { public: bool submit(const MaintenanceRobotSystemCommand&); const MaintenanceRobotSystemState* find(std::uint64_t) const; std::vector<MaintenanceRobotSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,MaintenanceRobotSystemState> map_; };
}
