#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Resolve mounted ballistic, beam, missile, mining, point-defense, and utility weapons through common combat events.
struct VehicleWeaponSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct VehicleWeaponSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class VehicleWeaponSystemSystem { public: bool submit(const VehicleWeaponSystemCommand&); const VehicleWeaponSystemState* find(std::uint64_t) const; std::vector<VehicleWeaponSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,VehicleWeaponSystemState> map_; };
}
