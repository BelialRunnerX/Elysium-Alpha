#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track pads, hangars, customs, cargo, passenger flows, fuel, maintenance, traffic control, and emergency response.
struct SpaceportSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct SpaceportSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class SpaceportSystemSystem { public: bool submit(const SpaceportSystemCommand&); const SpaceportSystemState* find(std::uint64_t) const; std::vector<SpaceportSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SpaceportSystemState> map_; };
}
