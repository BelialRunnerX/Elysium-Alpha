#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent pulsar timing, radiation cones, sensor opportunities, hazards, and energy/research exploitation.
struct PulsarSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct PulsarSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class PulsarSystemSystem { public: bool submit(const PulsarSystemCommand&); const PulsarSystemState* find(std::uint64_t) const; std::vector<PulsarSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,PulsarSystemState> map_; };
}
