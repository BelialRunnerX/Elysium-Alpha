#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Coordinate aid fleets for famine, disaster, plague, siege, refugees, and infrastructure collapse.
struct ReliefMissionSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct ReliefMissionSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class ReliefMissionSystemSystem { public: bool submit(const ReliefMissionSystemCommand&); const ReliefMissionSystemState* find(std::uint64_t) const; std::vector<ReliefMissionSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ReliefMissionSystemState> map_; };
}
