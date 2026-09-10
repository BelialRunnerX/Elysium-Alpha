#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate limited raids against logistics, infrastructure, research, settlements, and fleets without full occupation goals.
struct StrategicRaidSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct StrategicRaidSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class StrategicRaidSystemSystem { public: bool submit(const StrategicRaidSystemCommand&); const StrategicRaidSystemState* find(std::uint64_t) const; std::vector<StrategicRaidSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,StrategicRaidSystemState> map_; };
}
