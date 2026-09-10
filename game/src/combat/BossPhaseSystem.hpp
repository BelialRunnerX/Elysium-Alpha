#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent deterministic boss phase transitions from health, objectives, environment, adds, timers, and scripted facts.
struct BossPhaseSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct BossPhaseSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class BossPhaseSystemSystem { public: bool submit(const BossPhaseSystemCommand&); const BossPhaseSystemState* find(std::uint64_t) const; std::vector<BossPhaseSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,BossPhaseSystemState> map_; };
}
