#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent cargo rail lines, stations, trains, manifests, capacity, schedule, reservations, and blockage events.
struct CargoRailSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CargoRailSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CargoRailSystemStore { public: bool apply(const CargoRailSystemOp&); bool erase(std::uint64_t); const CargoRailSystemData* find(std::uint64_t) const; std::vector<CargoRailSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CargoRailSystemData> data_; };
}
