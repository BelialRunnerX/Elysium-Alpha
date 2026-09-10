#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Aggregate sparse mines, shafts, extractors, processing hubs, rail, power, depletion, and workforce across a planet.
struct PlanetaryMiningNetworkCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct PlanetaryMiningNetworkState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class PlanetaryMiningNetworkSystem { public: bool submit(const PlanetaryMiningNetworkCommand&); const PlanetaryMiningNetworkState* find(std::uint64_t) const; std::vector<PlanetaryMiningNetworkState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,PlanetaryMiningNetworkState> map_; };
}
