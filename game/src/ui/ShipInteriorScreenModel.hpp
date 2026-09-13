#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build compartment pressure, power, heat, damage, crew, doors, fires, repairs, and boarding projections.
struct ShipInteriorScreenModelCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct ShipInteriorScreenModelState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class ShipInteriorScreenModelSystem { public: bool submit(const ShipInteriorScreenModelCommand&); const ShipInteriorScreenModelState* find(std::uint64_t) const; std::vector<ShipInteriorScreenModelState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ShipInteriorScreenModelState> map_; };
}
