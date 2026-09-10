#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate reactor, engines, alarms, weapons, impacts, decompression, machinery, and crew ambience events for ships.
struct ShipAudioSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct ShipAudioSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class ShipAudioSystemSystem { public: bool submit(const ShipAudioSystemCommand&); const ShipAudioSystemState* find(std::uint64_t) const; std::vector<ShipAudioSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ShipAudioSystemState> map_; };
}
