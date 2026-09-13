#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Coordinate late-game stable Rift gate research, construction, power, calibration, defense, and destination-lock milestones.
struct RiftGateProjectCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct RiftGateProjectState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class RiftGateProjectSystem { public: bool submit(const RiftGateProjectCommand&); const RiftGateProjectState* find(std::uint64_t) const; std::vector<RiftGateProjectState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,RiftGateProjectState> map_; };
}
