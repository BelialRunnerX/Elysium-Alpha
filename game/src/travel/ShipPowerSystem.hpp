#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track reactors, batteries, buses, breakers, priority loads, overload, damage, and emergency redistribution aboard ships.
struct ShipPowerSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct ShipPowerSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class ShipPowerSystemSystem { public: bool submit(const ShipPowerSystemCommand&); const ShipPowerSystemState* find(std::uint64_t) const; std::vector<ShipPowerSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ShipPowerSystemState> map_; };
}
