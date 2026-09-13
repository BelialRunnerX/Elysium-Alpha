#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build renderer-neutral distortion, emissive, particle, geometry, stability, hazard, and portal destination presentation records.
struct RiftRenderModelCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct RiftRenderModelState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class RiftRenderModelSystem { public: bool submit(const RiftRenderModelCommand&); const RiftRenderModelState* find(std::uint64_t) const; std::vector<RiftRenderModelState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,RiftRenderModelState> map_; };
}
