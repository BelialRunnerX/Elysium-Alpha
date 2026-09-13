#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Resolve strategic/local ship weapon firing intents, arcs, power, heat, ammunition, targets, and damage payloads.
struct ShipWeaponSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct ShipWeaponSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class ShipWeaponSystemSystem { public: bool submit(const ShipWeaponSystemCommand&); const ShipWeaponSystemState* find(std::uint64_t) const; std::vector<ShipWeaponSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ShipWeaponSystemState> map_; };
}
