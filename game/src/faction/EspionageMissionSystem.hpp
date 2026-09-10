#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track spy missions for reconnaissance, sabotage, theft, rescue, propaganda, infiltration, and counterintelligence.
struct EspionageMissionSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct EspionageMissionSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class EspionageMissionSystemSystem { public: bool submit(const EspionageMissionSystemCommand&); const EspionageMissionSystemState* find(std::uint64_t) const; std::vector<EspionageMissionSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,EspionageMissionSystemState> map_; };
}
