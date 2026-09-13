#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Aggregate crew, fuel, ammunition, damage, maintenance, supplies, morale, and command readiness for fleets.
struct FleetReadinessSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct FleetReadinessSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class FleetReadinessSystemStore { public: bool apply(const FleetReadinessSystemOp&); bool erase(std::uint64_t); const FleetReadinessSystemData* find(std::uint64_t) const; std::vector<FleetReadinessSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,FleetReadinessSystemData> data_; };
}
