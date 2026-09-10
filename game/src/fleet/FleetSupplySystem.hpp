#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track fleet fuel, munitions, food, parts, medicine, transfer operations, tenders, and resupply thresholds.
struct FleetSupplySystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct FleetSupplySystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class FleetSupplySystemStore { public: bool apply(const FleetSupplySystemOp&); bool erase(std::uint64_t); const FleetSupplySystemData* find(std::uint64_t) const; std::vector<FleetSupplySystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,FleetSupplySystemData> data_; };
}
