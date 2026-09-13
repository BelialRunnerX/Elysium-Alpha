#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent planetary shield generators, relays, energy storage, coverage, damage, maintenance, and strategic effects.
struct PlanetaryShieldProjectCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct PlanetaryShieldProjectState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class PlanetaryShieldProjectSystem { public: bool submit(const PlanetaryShieldProjectCommand&); const PlanetaryShieldProjectState* find(std::uint64_t) const; std::vector<PlanetaryShieldProjectState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,PlanetaryShieldProjectState> map_; };
}
