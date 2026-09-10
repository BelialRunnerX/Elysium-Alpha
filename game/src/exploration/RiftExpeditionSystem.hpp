#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track staged Rift expeditions with stability, exposure, navigation uncertainty, anomalies, resources, and return risk.
struct RiftExpeditionSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct RiftExpeditionSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class RiftExpeditionSystemStore { public: bool apply(const RiftExpeditionSystemOp&); bool erase(std::uint64_t); const RiftExpeditionSystemData* find(std::uint64_t) const; std::vector<RiftExpeditionSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,RiftExpeditionSystemData> data_; };
}
