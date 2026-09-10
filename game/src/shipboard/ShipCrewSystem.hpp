#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track shipboard roles, watches, quarters, fatigue, morale, qualifications, emergency stations, and command hierarchy.
struct ShipCrewSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ShipCrewSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ShipCrewSystemStore { public: bool apply(const ShipCrewSystemOp&); bool erase(std::uint64_t); const ShipCrewSystemData* find(std::uint64_t) const; std::vector<ShipCrewSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ShipCrewSystemData> data_; };
}
