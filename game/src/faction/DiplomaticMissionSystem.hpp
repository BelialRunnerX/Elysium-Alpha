#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track embassies, envoys, summits, negotiations, incidents, guarantees, and communication delay.
struct DiplomaticMissionSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct DiplomaticMissionSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class DiplomaticMissionSystemSystem { public: bool submit(const DiplomaticMissionSystemCommand&); const DiplomaticMissionSystemState* find(std::uint64_t) const; std::vector<DiplomaticMissionSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,DiplomaticMissionSystemState> map_; };
}
