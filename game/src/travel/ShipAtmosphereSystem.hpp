#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track compartment oxygen, pressure, contaminants, leaks, vents, scrubbers, fires, and emergency isolation aboard ships.
struct ShipAtmosphereSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct ShipAtmosphereSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class ShipAtmosphereSystemSystem { public: bool submit(const ShipAtmosphereSystemCommand&); const ShipAtmosphereSystemState* find(std::uint64_t) const; std::vector<ShipAtmosphereSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ShipAtmosphereSystemState> map_; };
}
