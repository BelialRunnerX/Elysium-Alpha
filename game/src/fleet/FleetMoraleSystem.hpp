#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Aggregate crew morale, fatigue, victories, losses, leadership, supplies, home contact, and mutiny risk.
struct FleetMoraleSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct FleetMoraleSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class FleetMoraleSystemStore { public: bool apply(const FleetMoraleSystemOp&); bool erase(std::uint64_t); const FleetMoraleSystemData* find(std::uint64_t) const; std::vector<FleetMoraleSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,FleetMoraleSystemData> data_; };
}
