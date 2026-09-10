#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Move aggregate population cohorts between settlements/systems based on policy, demand, safety, and transport capacity.
struct StrategicMigrationOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct StrategicMigrationData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class StrategicMigrationStore { public: bool apply(const StrategicMigrationOp&); bool erase(std::uint64_t); const StrategicMigrationData* find(std::uint64_t) const; std::vector<StrategicMigrationData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,StrategicMigrationData> data_; };
}
