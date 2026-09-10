#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate unbound worlds with thermal state, resources, hazards, hidden sites, and discovery difficulty.
struct RoguePlanetSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct RoguePlanetSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class RoguePlanetSystemSystem { public: bool submit(const RoguePlanetSystemCommand&); const RoguePlanetSystemState* find(std::uint64_t) const; std::vector<RoguePlanetSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,RoguePlanetSystemState> map_; };
}
