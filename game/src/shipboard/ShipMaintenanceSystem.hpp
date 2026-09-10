#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track preventive/corrective maintenance, spares, access, downtime, crew skills, and subsystem readiness.
struct ShipMaintenanceSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ShipMaintenanceSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ShipMaintenanceSystemStore { public: bool apply(const ShipMaintenanceSystemOp&); bool erase(std::uint64_t); const ShipMaintenanceSystemData* find(std::uint64_t) const; std::vector<ShipMaintenanceSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ShipMaintenanceSystemData> data_; };
}
