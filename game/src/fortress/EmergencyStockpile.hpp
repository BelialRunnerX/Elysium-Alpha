#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track emergency reserve targets, freshness, accessibility, distribution, and replenishment policies.
struct EmergencyStockpileOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct EmergencyStockpileData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class EmergencyStockpileStore { public: bool apply(const EmergencyStockpileOp&); bool erase(std::uint64_t); const EmergencyStockpileData* find(std::uint64_t) const; std::vector<EmergencyStockpileData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,EmergencyStockpileData> data_; };
}
