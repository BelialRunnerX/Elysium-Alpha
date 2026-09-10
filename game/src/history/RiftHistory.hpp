#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Record Rift discoveries, disasters, expeditions, gates, artifacts, contamination, and civilization-changing events.
struct RiftHistoryCommand { std::uint64_t subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; };
struct RiftHistoryState { std::uint64_t revision{0}, subject{0}, owner{0}, target{0}; double strength{0.0}; std::uint32_t mode{0}; bool active{false}; };
class RiftHistorySystem { public: bool submit(const RiftHistoryCommand&); const RiftHistoryState* find(std::uint64_t) const; std::vector<RiftHistoryState> states() const; bool cancel(std::uint64_t); void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,RiftHistoryState> map_; };
}
