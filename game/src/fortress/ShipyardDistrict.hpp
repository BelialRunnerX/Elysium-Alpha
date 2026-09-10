#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track drydocks, fabrication bays, module storage, workforce, power, logistics, and ship-construction queues.
struct ShipyardDistrictCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct ShipyardDistrictState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class ShipyardDistrictSystem { public: bool submit(const ShipyardDistrictCommand&); const ShipyardDistrictState* find(std::uint64_t) const; std::vector<ShipyardDistrictState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ShipyardDistrictState> map_; };
}
