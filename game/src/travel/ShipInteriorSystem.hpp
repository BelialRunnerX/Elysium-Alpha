#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent ship compartment graphs, pressure zones, doors, utilities, damage, crew stations, and boarding topology.
struct ShipInteriorSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct ShipInteriorSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class ShipInteriorSystemSystem { public: bool submit(const ShipInteriorSystemCommand&); const ShipInteriorSystemState* find(std::uint64_t) const; std::vector<ShipInteriorSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ShipInteriorSystemState> map_; };
}
