#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track guided missile target locks, fuel, seeker state, countermeasures, interception, proximity detonation, and warheads.
struct MissileSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct MissileSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class MissileSystemSystem { public: bool submit(const MissileSystemCommand&); const MissileSystemState* find(std::uint64_t) const; std::vector<MissileSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,MissileSystemState> map_; };
}
