#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate layered anomaly/Rift ambience, instability, traversal, contamination, artifact, and collapse audio intents.
struct RiftAudioSystemCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct RiftAudioSystemState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class RiftAudioSystemSystem { public: bool submit(const RiftAudioSystemCommand&); const RiftAudioSystemState* find(std::uint64_t) const; std::vector<RiftAudioSystemState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,RiftAudioSystemState> map_; };
}
