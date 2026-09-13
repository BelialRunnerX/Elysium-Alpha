#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track access zones, armories, brig, boarding alarms, patrols, cameras/sensors, and lockdown orders.
struct ShipSecuritySystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ShipSecuritySystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ShipSecuritySystemStore { public: bool apply(const ShipSecuritySystemOp&); bool erase(std::uint64_t); const ShipSecuritySystemData* find(std::uint64_t) const; std::vector<ShipSecuritySystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ShipSecuritySystemData> data_; };
}
