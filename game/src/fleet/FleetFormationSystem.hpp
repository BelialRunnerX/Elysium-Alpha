#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent formation slots, roles, spacing, escorts, screens, reserves, and maneuver doctrine.
struct FleetFormationSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct FleetFormationSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class FleetFormationSystemStore { public: bool apply(const FleetFormationSystemOp&); bool erase(std::uint64_t); const FleetFormationSystemData* find(std::uint64_t) const; std::vector<FleetFormationSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,FleetFormationSystemData> data_; };
}
