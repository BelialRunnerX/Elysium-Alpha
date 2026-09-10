#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Plan remote stations, relays, colonies, gates, defenses, shipyards, and infrastructure from aggregate resources.
struct StrategicConstructionOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct StrategicConstructionData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class StrategicConstructionStore { public: bool apply(const StrategicConstructionOp&); bool erase(std::uint64_t); const StrategicConstructionData* find(std::uint64_t) const; std::vector<StrategicConstructionData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,StrategicConstructionData> data_; };
}
