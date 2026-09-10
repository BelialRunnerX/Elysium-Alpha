#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track field repair capacity, shipyard requirements, spare parts, repair queues, and return-to-service estimates.
struct FleetRepairSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct FleetRepairSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class FleetRepairSystemStore { public: bool apply(const FleetRepairSystemOp&); bool erase(std::uint64_t); const FleetRepairSystemData* find(std::uint64_t) const; std::vector<FleetRepairSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,FleetRepairSystemData> data_; };
}
