#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate meteor showers/impacts with trajectories, local damage, resources, hazards, and Chronicle hooks.
struct MeteorEventSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct MeteorEventSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class MeteorEventSystemStore { public: bool apply(const MeteorEventSystemOp&); bool erase(std::uint64_t); const MeteorEventSystemData* find(std::uint64_t) const; std::vector<MeteorEventSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,MeteorEventSystemData> data_; };
}
