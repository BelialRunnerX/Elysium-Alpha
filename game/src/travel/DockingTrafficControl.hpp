#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Schedule ship docking/undocking slots, holds, emergencies, customs, quarantine, and service priorities.
struct DockingTrafficControlOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct DockingTrafficControlData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class DockingTrafficControlStore { public: bool apply(const DockingTrafficControlOp&); bool erase(std::uint64_t); const DockingTrafficControlData* find(std::uint64_t) const; std::vector<DockingTrafficControlData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,DockingTrafficControlData> data_; };
}
