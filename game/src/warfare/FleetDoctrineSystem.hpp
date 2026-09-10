#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Store faction fleet doctrine preferences for formation, engagement, retreat, escort, interdiction, and resupply.
struct FleetDoctrineSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct FleetDoctrineSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class FleetDoctrineSystemStore { public: bool apply(const FleetDoctrineSystemOp&); bool erase(std::uint64_t); const FleetDoctrineSystemData* find(std::uint64_t) const; std::vector<FleetDoctrineSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,FleetDoctrineSystemData> data_; };
}
