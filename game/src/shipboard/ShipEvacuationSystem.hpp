#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Plan lifeboats, escape pods, suits, rally points, triage, passenger priorities, and abandonment state.
struct ShipEvacuationSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ShipEvacuationSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ShipEvacuationSystemStore { public: bool apply(const ShipEvacuationSystemOp&); bool erase(std::uint64_t); const ShipEvacuationSystemData* find(std::uint64_t) const; std::vector<ShipEvacuationSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ShipEvacuationSystemData> data_; };
}
