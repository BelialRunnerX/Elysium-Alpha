#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent wells, heat exchangers, generation, reservoir pressure, induced seismicity, and maintenance.
struct GeothermalNetworkCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct GeothermalNetworkState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class GeothermalNetworkSystem { public: bool submit(const GeothermalNetworkCommand&); const GeothermalNetworkState* find(std::uint64_t) const; std::vector<GeothermalNetworkState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,GeothermalNetworkState> map_; };
}
