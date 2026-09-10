#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent sparse authored roads/tracks by stable endpoints, surface addresses, condition, ownership, and traversal modifiers.
struct RoadNetworkOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct RoadNetworkData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class RoadNetworkStore { public: bool apply(const RoadNetworkOp&); bool erase(std::uint64_t); const RoadNetworkData* find(std::uint64_t) const; std::vector<RoadNetworkData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,RoadNetworkData> data_; };
}
