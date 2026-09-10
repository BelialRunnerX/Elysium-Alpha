#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent ship interior compartments, doors, pressure, hazards, access, occupants, damage, and local services.
struct CompartmentSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CompartmentSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CompartmentSystemStore { public: bool apply(const CompartmentSystemOp&); bool erase(std::uint64_t); const CompartmentSystemData* find(std::uint64_t) const; std::vector<CompartmentSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CompartmentSystemData> data_; };
}
