#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build fleets, readiness, doctrine, supply, formations, missions, threats, repair, and command intent projections.
struct FleetCommandScreenModelOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct FleetCommandScreenModelData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class FleetCommandScreenModelStore { public: bool apply(const FleetCommandScreenModelOp&); bool erase(std::uint64_t); const FleetCommandScreenModelData* find(std::uint64_t) const; std::vector<FleetCommandScreenModelData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,FleetCommandScreenModelData> data_; };
}
