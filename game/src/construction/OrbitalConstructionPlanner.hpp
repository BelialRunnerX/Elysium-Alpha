#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Plan staged orbital platforms, stations, elevators, docks, solar arrays, and shipyard expansions.
struct OrbitalConstructionPlannerOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct OrbitalConstructionPlannerData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class OrbitalConstructionPlannerStore { public: bool apply(const OrbitalConstructionPlannerOp&); bool erase(std::uint64_t); const OrbitalConstructionPlannerData* find(std::uint64_t) const; std::vector<OrbitalConstructionPlannerData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,OrbitalConstructionPlannerData> data_; };
}
